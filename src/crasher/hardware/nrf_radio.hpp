#ifndef NRF_RADIO_H
#define NRF_RADIO_H

#include <cstdio>
#include <cstring>
#include <wrappers/wmutex.h>
#include "error_types.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_802154.h"

/**
 * Radio frame:
 *  name    | mac header | source_addr | dest_addr | command | current chunk | total chunks | payload_len | payload | suffix( rssi&lqi )
 *  size    |  2 bytes   |  8 bytes    |  8 bytes  | 2 bytes |    1 byte     |    1 byte    |   1 byte    | x bytes |    2 bytes
 *
 *  Max frame size is limited by nrf_drv_radio802154 to 127 bytes (so RF_PAYLOAD_SIZE is based on that limit)
 *  Header, suffix and source address are set up automatically in NrfRadio::Transmit().
 */

#define RF_HEADER_LEN                     2
#define RF_SEQNUM_LEN                     1
#define RF_ADDR_LEN                       8
#define RF_CMD_LEN                        2
#define RF_PANID_LEN                      2
#define RF_CURRENT_CHUNK_SIZE             sizeof(uint8_t)
#define RF_TOTAL_CHUNKS_SIZE              sizeof(uint8_t)
#define RF_PAYLOAD_LEN_SIZE               sizeof(uint8_t)
#define RF_SUFFIX_LEN                     2

#define RF_CONFIG_LEN                     ((RF_HEADER_LEN)+RF_SEQNUM_LEN+RF_PANID_LEN+((RF_ADDR_LEN)*2)+(RF_CMD_LEN)+(RF_CURRENT_CHUNK_SIZE)+(RF_TOTAL_CHUNKS_SIZE)+(RF_PAYLOAD_LEN_SIZE)+(RF_SUFFIX_LEN))

#define RF_PAYLOAD_LEN                    (128-(RF_CONFIG_LEN))

struct __attribute__((__packed__)) RadioFrame {
  uint8_t   header[RF_HEADER_LEN];
  uint8_t   seqNumber;
  uint8_t   destPanId[RF_PANID_LEN];
  uint8_t   destAddr[RF_ADDR_LEN];
  uint8_t   sourceAddr[RF_ADDR_LEN];
  uint8_t   command[RF_CMD_LEN];
  uint8_t   currentChunk;
  uint8_t   totalChunks;
  uint8_t   payloadLength;
  uint8_t   payload[RF_PAYLOAD_LEN];
};

enum class RadioState {
  SLEEP,
  RECEIVE
};

class NrfRadio {
public:
  static Wrappers::Semaphore mReceiveSemaphore;
  static Wrappers::Semaphore mTransmitSemaphore;
  static Wrappers::Mutex mAddrMutex;
  static Wrappers::Mutex mRadioMutex;

  static void Init();
  static ErrorType SetState(RadioState state);

  static ErrorType Transmit(uint8_t *destAddr, uint8_t *payload, uint32_t payloadLen, uint8_t *command, uint16_t timeout, bool ack);
  static ErrorType TransmitRaw(RadioFrame *frame, uint16_t timeout, uint8_t seqNum, bool ack, bool morePending, bool unicast);

  static ErrorType Receive(uint8_t *sourceAddr, uint8_t *payload, uint32_t *payloadLen, uint8_t *command, uint16_t timeout, int8_t *rssi);
  static ErrorType ReceiveRaw(RadioFrame *frame, uint16_t timeout, int8_t *rssi);

  static bool SetAddr(uint8_t *newAddr, int len);
  static void GetAddr(uint8_t *destAddr, int *len);

  static ErrorType SetChannel(uint8_t channel);
  static void FrameReceived(uint8_t *p_data, uint8_t length, int8_t power, uint8_t lqi);
  static void FrameTransmitted(uint8_t *p_data, uint8_t length, int8_t power, uint8_t lqi);

};

#endif