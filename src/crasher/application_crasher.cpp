#include "application_crasher.hpp"
#include "nrf_drv_gpiote.h"

using namespace Crasher;
using namespace Wrappers;

Wrappers::Task Application::mTaskReadCommand;
Wrappers::Task Application::mListenerTask;
Wrappers::Task Application::mTaskLogger;

void Application::Run(int argc, char **argv) {
    if (mTaskReadCommand.Create(ReadCommandTask, this, "READ-COMMAND", DEFAULT_TASK_STACK_SIZE, DEFAULT_TASK_PRIORITY,
                                false) != Task::Status::NO_ERROR) {
      NRF_LOG_ERROR("Error while creating read command task");
    } else {
//      NRF_LOG_INFO("Read command task created");
    }

    if (mListenerTask.Create(ListenerTask, this, "LISTENER", DEFAULT_TASK_STACK_SIZE, DEFAULT_TASK_PRIORITY,
                                false) != Task::Status::NO_ERROR) {
      NRF_LOG_ERROR("Error while creating listener task");
    } else {
//      NRF_LOG_INFO("Listener task created");
    }

    #if NRF_LOG_ENABLED && NRF52840_XXAA && NRF_LOG_DEFERRED
    if( mTaskLogger.Create(LoggerTask, this, "LOGGER") != Task::Status::NO_ERROR ) {
        NRF_LOG_ERROR("Error while creating logger task");
    } else {
//        NRF_LOG_INFO("Logger task created: %p", &mTaskLogger);
    }
#endif //NRF_LOG_ENABLED

}

static RadioFrame frame;

void Application::ListenerTask() {

//  NRF_LOG_INFO("Listening");
//  int cnt = 0;
  while (true) {
      int8_t rssi;
      NrfRadio::ReceiveRaw(&frame, 1000, &rssi );
      mTaskReadCommand.DelayUntil(10UL);
//      NRF_LOG_INFO("Listening %d", cnt++);
//      if( cnt == 10 ) {
//          NVIC_SystemReset();
//      }
  }
}

void Application::ReadCommandTask() {

//  NRF_LOG_INFO("Waiting");
  while (true) {
      TickType_t mLastWakeTime = xTaskGetTickCount();
      TickType_t org = mLastWakeTime;
      vTaskDelayUntil(&mLastWakeTime, pdMS_TO_TICKS( 10UL ));
      TickType_t cur = xTaskGetTickCount();
      if( ((mLastWakeTime - org) != 10) || ((cur - org) != 10) ) {
          NRF_LOG_INFO("SLEEP l-o=%d l=%d c-o=%d c=%d o=%d", mLastWakeTime - org, mLastWakeTime, cur-org, cur, org);
      }
  }
}

#if NRF_LOG_ENABLED && NRF52840_XXAA && NRF_LOG_DEFERRED

static uint32_t lastFlush = 0;
void Application::LoggerTask()
{
//      TickType_t last = xTaskGetTickCount();
    while( true ) {
//          TickType_t cur = xTaskGetTickCount();

          if( lastFlush > 1 ) {
        NRF_LOG_INFO("FLUSH %d", lastFlush);
//        last = cur;
        }
      TickType_t last = xTaskGetTickCount();
        NRF_LOG_FLUSH();
      TickType_t cur = xTaskGetTickCount();
      lastFlush = cur - last;
        Task::SuspendCurrent();
    }
}

#endif

extern "C" void vApplicationIdleHook(void);

void vApplicationIdleHook(void) {
#if NRF_LOG_ENABLED && NRF52840_XXAA && NRF_LOG_DEFERRED
    Application::mTaskLogger.Resume();
#endif

}

extern "C" void vAssertCalled(unsigned long ulLine, const char *const pcFileName);

void vAssertCalled(unsigned long ulLine, const char *const pcFileName) {
  UNUSED_PARAMETER(pcFileName);
  UNUSED_PARAMETER(ulLine);

  NRF_LOG_INFO("ASSERT: line: %d, file: %s", (int) ulLine, pcFileName);

  taskDISABLE_INTERRUPTS();
  for (;;);
}

extern "C" void vApplicationMallocFailedHook(void);

void vApplicationMallocFailedHook(void) {
  NRF_LOG_INFO("Malloc failed!");
  while (true);
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t *aTaskPtr, signed char *aTaskNamePtr);

void vApplicationStackOverflowHook(TaskHandle_t *aTaskPtr, signed char *aTaskNamePtr) {
  UNUSED_PARAMETER(aTaskPtr);
  UNUSED_PARAMETER(aTaskNamePtr);

  NRF_LOG_INFO("Stack overflow! Task name: %s", aTaskNamePtr);
}
