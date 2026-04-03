#include "hal/poll_manager.h"
#include "drivers/gpio_driver.h"
#include "drivers/gpio_expander_driver.h"
#include "drivers/gpio_expander_pcf8574.h"
#include "drivers/switch.h"

hal::Pcf8574Expander expander(0x20);

hal::ExpanderGpioDriver p0(0, hal::GpioConfig::kInputPullup, expander);
hal::SwitchDriver testSwitch(p0, SwitchId::kBypass);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("Switch test starting...");
  expander.init();
  testSwitch.init();

  Serial.println("Setup done main loop starting...");
}

void loop() {
  Event events[4];
  size_t eventsCount = 0;

  eventsCount = expander.poll(events, 4);
  eventsCount = testSwitch.poll(events, 4);

  for (uint8_t i = 0; i < eventsCount; i++) {
    Serial.print("Received event : ");
    Serial.println(static_cast<uint8_t>(events[i].m_type));
    Serial.print("Delta : ");
    Serial.println(events[i].m_data.delta);
  }
}
