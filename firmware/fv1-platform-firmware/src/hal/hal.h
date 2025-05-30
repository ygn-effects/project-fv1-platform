#pragma once

#include "drivers/bypass_driver.h"
#include "drivers/encoder.h"
#include "drivers/eeprom_m95.h"
#include "drivers/gpio_driver.h"
#include "drivers/gpio_expander_driver.h"
#include "drivers/gpio_expander_pcf8574.h"
#include "drivers/led.h"
#include "drivers/potentiometer.h"
#include "drivers/switch.h"
#include "hal/poll_manager.h"

namespace hal {

extern M95Driver eeprom;
extern Bypass bypass;
extern LedDriver programModeLed;
extern LedDriver bypassSwitchLed;
extern LedDriver tapSwitchLed;

void init();
void update();

}