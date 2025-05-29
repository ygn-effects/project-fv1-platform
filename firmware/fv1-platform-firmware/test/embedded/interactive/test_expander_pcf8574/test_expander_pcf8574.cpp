#include "hal/poll_manager.h"
#include "drivers/gpio_driver.h"
#include "drivers/gpio_expander_driver.h"
#include "drivers/gpio_expander_pcf8574.h"

hal::Pcf8574Expander expander(0x20);

hal::ExpanderGpioDriver p0(0, hal::GpioConfig::kInputPullup, expander);
hal::ExpanderGpioDriver p1(1, hal::GpioConfig::kInputPullup, expander);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("GPIO Expander test starting...");

  expander.init();
  p0.init();
  p1.init();

  Event events[4];
  expander.poll(events, 4);

  Serial.println(p0.read());
  Serial.println(p1.read());

  Serial.println("Setup done main loop starting...");
}

void loop() {

}
