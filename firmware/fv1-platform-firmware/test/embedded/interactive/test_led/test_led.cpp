#include "hal/poll_manager.h"
#include "drivers/gpio_driver.h"
#include "drivers/led.h"

hal::DigitalGpioDriver testLedGpio(1, hal::GpioConfig::kInputPullup);
hal::LedDriver testLed(testLedGpio);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("LED test starting...");
  testLed.init();

  Serial.println("LED on");
  testLed.on();

  delay(2000);

  Serial.println("LED off");
  testLed.off();

  delay(2000);

  Serial.println("Setup done main loop starting...");
}

void loop() {
  testLed.toggle();
  delay(1000);
}
