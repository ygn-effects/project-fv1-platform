#include "hal/poll_manager.h"
#include "drivers/gpio_driver.h"
#include "drivers/gpio_expander_driver.h"
#include "drivers/gpio_expander_pcf8574.h"
#include "drivers/led.h"

hal::Pcf8574Expander expander(0x20);

hal::ExpanderGpioDriver p0(0, hal::GpioConfig::kOutput, expander);
hal::BlinkingLedDriver testLed(p0, 1000);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("Encoder test starting...");
  expander.init();
  testLed.init();

  Serial.println("Setup done main loop starting...");
}

void loop() {
  testLed.update(millis());
}
