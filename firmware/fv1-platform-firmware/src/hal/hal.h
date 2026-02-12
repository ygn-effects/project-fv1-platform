#pragma once

#include "drivers/bypass_driver.h"
#include "drivers/clock_driver.h"
#include "drivers/dac_mcp49xx.h"
#include "drivers/display_ssd1306.h"
#include "drivers/encoder.h"
#include "drivers/eeprom_m95.h"
#include "drivers/fv1_driver.h"
#include "drivers/gpio_driver.h"
#include "drivers/gpio_expander_driver.h"
#include "drivers/gpio_expander_pcf8574.h"
#include "drivers/led.h"
#include "drivers/potentiometer.h"
#include "drivers/serial_driver.h"
#include "drivers/switch.h"
#include "hal/poll_manager.h"

namespace hal {

extern ArduinoClock clock;
extern M95Driver eeprom;
extern Bypass bypass;
extern SSD1306Driver display;
extern LedDriver programModeLed;
extern LedDriver menuLockLed;
extern LedDriver bypassSwitchLed;
extern AdjustableLedDriver tapSwitchLed;
extern SerialDriver midiSerial;
extern Fv1Driver fv1;
extern Mcp4912 vcaDryDac;
extern Mcp4912 vcaWetDac;

void init();
void update();

}