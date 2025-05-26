#include "hal/poll_manager.h"
#include "drivers/switch.h"

hal::DigitalGpioDriver testSwitchGpio(1, hal::GpioConfig::kInputPullup);
hal::SwitchDriver testSwitch(testSwitchGpio, SwitchId::kBypass);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("Switch test starting...");
  testSwitch.init();

  Serial.println("Setup done main loop starting...");
}

void loop() {
  Event events[4];
  size_t eventsCount = 0;

  eventsCount = testSwitch.poll(events, 4);

  for (uint8_t i = 0; i < eventsCount; i++) {
    Serial.print("Received event : ");
    Serial.println(static_cast<uint8_t>(events[i].m_type));
  }
}
