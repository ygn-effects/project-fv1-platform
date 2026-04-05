#include "hal/poll_manager.h"
#include "drivers/gpio_driver.h"
#include "drivers/led.h"

hal::DigitalGpioDriver testLedGpio(1, hal::GpioConfig::kInputPullup);
hal::BlinkingLedDriver testLed(testLedGpio, 1000);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("LED test starting...");
  testLed.init();

  Serial.println("Setup done main loop starting...");
}

void loop() {
  testLed.update(millis());
}
