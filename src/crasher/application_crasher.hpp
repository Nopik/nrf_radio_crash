#ifndef APPLICATION_CRASHER_H
#define APPLICATION_CRASHER_H

#include "wrappers/wtask.h"
#include "wrappers/wmutex.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_log.h"
#include "hardware/nrf_radio.hpp"

extern "C" void vApplicationIdleHook(void);

namespace Crasher {

  class Application {

  public:
      void Run(int argc, char **argv);

  private:
      friend void::vApplicationIdleHook();

/***************************************************************************************************
 * @section Tasks for Radio Commands
 **************************************************************************************************/

      static void ReadCommandTask(void *aContextPtr) { static_cast<Application *>(aContextPtr)->ReadCommandTask(); }
      void ReadCommandTask();

      static void ListenerTask(void *aContextPtr) { static_cast<Application *>(aContextPtr)->ListenerTask(); }
      void ListenerTask();

      static Wrappers::Task mTaskReadCommand;
      static Wrappers::Task mListenerTask;
  };

}

#endif