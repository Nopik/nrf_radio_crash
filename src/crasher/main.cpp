#include <cstdbool>
#include <cstdint>
#include "nrf_log.h"
#include "sdk_config.h"
#include "nrf_delay.h"
#include "FreeRTOS.h"
#include "boards.h"
#include "nrf_log_default_backends.h"
#include "nrf_radio.hpp"
#include "application_crasher.hpp"
#include "bsp.h"
//#include "common_utils.h"
//#include "platform/nrf52840/drivers/time_drv.h"

using namespace Crasher;

Application mAppData;

int main(int argc, char *argv[]) {
  bsp_init(BSP_INIT_LEDS, nullptr);
  bsp_board_led_on(0);

  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();

  NRF_LOG_INFO("\r\n");
  NRF_LOG_INFO("------------------------------------");
  NRF_LOG_INFO("Starting radio crasher");
  NRF_LOG_INFO("------------------------------------");
#if NRF_LOG_ENABLED
  NRF_LOG_PROCESS();
#endif //NRF_LOG_ENABLED

  NrfRadio::Init();

  mAppData.Run(argc, argv);

  /* Start FreeRTOS scheduler. */
  vTaskStartScheduler();

  NRF_LOG_INFO("Exit app ");
  return 0;
}
