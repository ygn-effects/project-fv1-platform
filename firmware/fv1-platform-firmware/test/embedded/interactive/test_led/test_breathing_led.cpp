#include "hal/poll_manager.h"
#include "drivers/gpio_driver.h"
#include "drivers/led.h"

hal::AnalogGpioDriver testLedGpio(15, hal::GpioConfig::kOutput);
hal::BreathingLedDriver testLed(testLedGpio, 100);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("LED test starting...");
  testLed.init();
  testLed.m_period = 150;

  Serial.println("Setup done main loop starting...");
}

void loop() {
  testLed.update();
}
