#include <crasher/error_types.h>
#include "nrf_radio.hpp"
#include "wmutex.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "boards.h"

#define TX_SINGLE_FRAME_BUFFER_SIZE     125
#define RX_SINGLE_FRAME_BUFFER_SIZE     127

#define CHANNEL_MIN_VAL                 11
#define CHANNEL_MAX_VAL                 26

#define RETRIES_COUNT                   10

using namespace Wrappers;

static uint8_t  mReceiverBuf[RX_SINGLE_FRAME_BUFFER_SIZE];
static uint8_t  mTransmitterBuf[TX_SINGLE_FRAME_BUFFER_SIZE];
static int8_t  mRssi;

static uint8_t mAddr[RF_ADDR_LEN] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

static RadioFrame mTransmitterSingleFrame;
static bool receiveBufferFull = false;

Wrappers::Semaphore NrfRadio::mReceiveSemaphore = Wrappers::Semaphore();
Wrappers::Semaphore NrfRadio::mTransmitSemaphore = Wrappers::Semaphore();
Wrappers::Mutex NrfRadio::mAddrMutex = Wrappers::Mutex();
Wrappers::Mutex NrfRadio::mRadioMutex = Wrappers::Mutex();

void NrfRadio::Init()
{
  nrf_802154_init();
  nrf_802154_tx_power_set(0);
  nrf_802154_channel_set(26);

  uint8_t panId[] = { 0xb1, 0x04 };
  nrf_802154_pan_id_set( panId );

#if defined(MASTER_NODE)
    uint8_t addr[] = { '1', '1', '1', '1', '1', '1', '1', '1' };
  NrfRadio::SetAddr(addr, sizeof(addr));
#else
    NrfRadio::SetAddr( (uint8_t *)( NRF_FICR->DEVICEID ), 8);
#endif
    SetState(RadioState::RECEIVE);
}

ErrorType NrfRadio::SetState(RadioState state){

  ErrorType err = ErrorType::INVALID_PARAMETERS;

  if(state == RadioState::SLEEP) {
    nrf_802154_sleep();
    if (nrf_802154_state_get() == NRF_802154_STATE_SLEEP) {
      err = ErrorType::NO_ERROR;
    } else {
      err = ErrorType::INTERNAL_ERROR;
    }
  }

  if(state == RadioState::RECEIVE) {
    nrf_802154_receive();
    if (nrf_802154_state_get() == NRF_802154_STATE_RECEIVE) {
      err = ErrorType::NO_ERROR;
    } else {
      err = ErrorType::INTERNAL_ERROR;
    }
  }

  return err;

}

bool NrfRadio::SetAddr(uint8_t *newAddr, int len){

  if(len == RF_ADDR_LEN){
    LockGuard guard(NrfRadio::mAddrMutex);
    memcpy(mAddr, newAddr, RF_ADDR_LEN);
    nrf_802154_extended_address_set( newAddr );
    return true;
  }else{
    return false;
  }
}

void NrfRadio::GetAddr(uint8_t *destAddr, int *len){
  LockGuard guard(NrfRadio::mAddrMutex);
  *len = RF_ADDR_LEN;
  memcpy(destAddr, mAddr, RF_ADDR_LEN);
}

ErrorType NrfRadio::TransmitRaw(RadioFrame *frame, uint16_t timeout, uint8_t seqNum, bool ack, bool morePending, bool unicast)
{
//  NRF_LOG_INFO("Transmit len: %d timeout %d", frame->payloadLength, timeout)

  memset(mTransmitterBuf, 0, TX_SINGLE_FRAME_BUFFER_SIZE);

  if( unicast ) {
    // set frame header for 802.15.4 driver
    frame->header[0] = 0x41;
    if (morePending) {
      frame->header[0] |= 0x10;
    }
    if (ack) {
      frame->header[0] |= 0x20;
    }

    frame->header[1] = 0xcc;
    frame->seqNumber = seqNum;
    frame->destPanId[0] = 0xb1;
    frame->destPanId[1] = 0x04;

    // copy addresses to frame
    memcpy(frame->sourceAddr, mAddr, RF_ADDR_LEN);
    memcpy(mTransmitterBuf, frame, sizeof(*frame));
  } else {
    frame->header[0] = 0x41;
    if (morePending) {
      frame->header[0] |= 0x10;
    }
    frame->header[1] = 0xc8;
    frame->destAddr[ 0 ] = 0xff;
    frame->destAddr[ 1 ] = 0xff;
    frame->seqNumber = seqNum;
    frame->destPanId[0] = 0xb1;
    frame->destPanId[1] = 0x04;

    memcpy(mTransmitterBuf, frame, 7 /* header + seq + panid + dest */ );
    memcpy(mTransmitterBuf + 7 , frame->sourceAddr, 127 - 13 /* offset of src addr */ );
  }

  for(uint8_t i=0;i<RETRIES_COUNT; i++){
    nrf_802154_transmit_csma_ca(mTransmitterBuf, sizeof( mTransmitterBuf ));
//    NRF_LOG_INFO("Transmitted data, try %d, timeout=%d", i, timeout );
//    NRF_LOG_HEXDUMP_INFO(mTransmitterBuf, sizeof(mTransmitterBuf));

    if( !ack ) {
      return ErrorType::NO_ERROR;
    }

    if(mTransmitSemaphore.Take(timeout)){
      return ErrorType::NO_ERROR;
    }
  }

  return ErrorType::INTERNAL_ERROR;
}

ErrorType NrfRadio::Transmit(uint8_t *destAddr, uint8_t *payload, uint32_t payloadLen, uint8_t *command, uint16_t timeout, bool ack){

  LockGuard guard(NrfRadio::mRadioMutex);

  ErrorType err;
  uint8_t totalChunks;
  uint8_t currentChunk;
  uint8_t currentChunkLen;
  uint32_t ptrOffset = 0;

  NrfRadio::SetState(RadioState::RECEIVE);

  if( destAddr != NULL ) {
    memcpy(mTransmitterSingleFrame.destAddr, destAddr, RF_ADDR_LEN);
  }
  memcpy(mTransmitterSingleFrame.command, command, RF_CMD_LEN);

  //// calculate chunks
  totalChunks = payloadLen/(RF_PAYLOAD_LEN);

  if((totalChunks * (RF_PAYLOAD_LEN)) < payloadLen){
    totalChunks++;
  }

  // if payload is empty, there should be one chunk to send
  if(totalChunks == 0){
    totalChunks = 1;
  }
//  NRF_LOG_INFO("Payload size: %d, total chunks: %d", payloadLen, totalChunks);

  mTransmitterSingleFrame.totalChunks = totalChunks;

  for( uint8_t i = 0; i<totalChunks; i++, ptrOffset += currentChunkLen){
    currentChunk = i+1;
    bool morePending;

    // check if it's last chunk
    if(i == (totalChunks-1)){
      currentChunkLen = payloadLen - ptrOffset;
      morePending = false;
    } else {
      currentChunkLen = RF_PAYLOAD_LEN;
      morePending = true;
    }

//    NRF_LOG_INFO("Sending chunk %d, current size: %d", currentChunk, currentChunkLen);

    mTransmitterSingleFrame.currentChunk = currentChunk;
    memcpy(mTransmitterSingleFrame.payload, payload+ptrOffset, currentChunkLen);
    mTransmitterSingleFrame.payloadLength = currentChunkLen;
    err = NrfRadio::TransmitRaw(&mTransmitterSingleFrame, 100, i, ack, morePending, destAddr != NULL );
//    NRF_LOG_INFO("TransmitRaw returned %d", err);

    if (err != ErrorType::NO_ERROR) {
      NRF_LOG_INFO("Chunk send error, exiting");
      return ErrorType::INTERNAL_ERROR;
    }
  }

//  NRF_LOG_INFO("All chunks sent");
  return ErrorType::NO_ERROR;
}

ErrorType NrfRadio::ReceiveRaw(RadioFrame *frame, uint16_t timeout, int8_t *rssi)
{

  int ret = 0;
  if(mReceiveSemaphore.Take(pdMS_TO_TICKS(timeout))){

    //copy to struct
    memcpy(frame, mReceiverBuf, RX_SINGLE_FRAME_BUFFER_SIZE);
    receiveBufferFull = false;
//    NRF_LOG_INFO("Received data below: ");
//    NRF_LOG_HEXDUMP_INFO(mReceiverBuf, sizeof(mReceiverBuf));

    //check destination address
    if( frame->header[1] == 0xcc ) {
      ret = memcmp(frame->destAddr, mAddr, RF_ADDR_LEN);
      if (ret == 0) {
        if (rssi != NULL) {
          *rssi = mRssi;
        }
        return ErrorType::NO_ERROR;
      }
    } else {
      if( frame->header[ 1 ] == 0xc8 ) {
        if ((frame->destAddr[0] == 0xff) && (frame->destAddr[1] == 0xff)) {
          if (rssi != NULL) {
            *rssi = mRssi;
          }
          return ErrorType::NO_ERROR;
        }
      }
    }
  }

//  NRF_LOG_INFO("Timeout - frame not received");
  return ErrorType::NOT_FOUND;
}



ErrorType NrfRadio::SetChannel(uint8_t channel){
  if((channel >= CHANNEL_MIN_VAL) && (channel <= CHANNEL_MAX_VAL)){
    nrf_802154_channel_set(channel);
    return ErrorType::NO_ERROR;
  }else{
    return ErrorType::INVALID_PARAMETERS;
  }
}

static int frameCnt = 0;

void NrfRadio::FrameReceived(uint8_t *p_data, uint8_t length, int8_t power, uint8_t lqi)
{

  (void) power;
  (void) lqi;

  if (length > RX_SINGLE_FRAME_BUFFER_SIZE)
  {
    NRF_LOG_INFO("Payload too big, dropping frame.")
    return;
  } else {

    bsp_board_led_invert(0);
      ++frameCnt;
//      if( (frameCnt % 10) == 0 ) {
//          NRF_LOG_INFO("Frame %d received, len: %d, rssi %d lqi %d dropping=%d", frameCnt, length, power, lqi, receiveBufferFull);
//      }

      if( frameCnt == 500 ) {
        NVIC_SystemReset();
      }

    if( receiveBufferFull == false ) {
      memcpy(mReceiverBuf, p_data, length);
      receiveBufferFull = true;
      mRssi = power;
    }

    nrf_802154_buffer_free(p_data);
    NrfRadio::mReceiveSemaphore.GiveFromISR();
  }
}

void NrfRadio::FrameTransmitted(uint8_t *p_ack, uint8_t length, int8_t power, uint8_t lqi)
{
  (void) power;
  (void) lqi;
  if (p_ack != NULL)
  {
    nrf_802154_buffer_free(p_ack);
  }
  NrfRadio::mTransmitSemaphore.GiveFromISR();

}

void nrf_802154_received_timestamp(uint8_t * p_data,
                                   uint8_t   length,
                                   int8_t    power,
                                   uint8_t   lqi,
                                   uint32_t  time){
  NRF_LOG_INFO("TS!!! rec");
}

void nrf_802154_transmitted_timestamp(const uint8_t * p_frame,
                                      uint8_t       * p_ack,
                                      uint8_t         length,
                                      int8_t          power,
                                      uint8_t         lqi,
                                      uint32_t        time){
  NRF_LOG_INFO("TS!!! tx");
}

void nrf_802154_transmit_failed(const uint8_t       * p_frame,
                                nrf_802154_tx_error_t error){
  NRF_LOG_INFO("TX FAIL %d", error);
}

void nrf_802154_receive_failed(nrf_802154_rx_error_t error){
  NRF_LOG_INFO("RX FAIL %d", error);
}
void nrf_802154_received(uint8_t *p_data, uint8_t length, int8_t power, uint8_t lqi) {
  NrfRadio::FrameReceived(p_data, length, power, lqi);
}

void nrf_802154_transmitted(const uint8_t * p_frame, uint8_t *p_ack, uint8_t length, int8_t power, uint8_t lqi) {
  NrfRadio::FrameTransmitted(p_ack, length, power, lqi);
}
