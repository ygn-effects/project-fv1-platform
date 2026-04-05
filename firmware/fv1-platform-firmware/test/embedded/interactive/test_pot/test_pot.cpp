#include "hal/poll_manager.h"
#include "drivers/potentiometer.h"

hal::AnalogGpioDriver testPotGpio(24, hal::GpioConfig::kInput);
hal::PotDriver testPot(testPotGpio, PotId::kPot0);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("Pot test starting...");
  testPot.init();

  Serial.println("Setup done main loop starting...");
}

void loop() {
  Event events[4];
  size_t eventsCount = 0;

  eventsCount = testPot.poll(events, 4);

  for (uint8_t i = 0; i < eventsCount; i++) {
    Serial.print("Received event : ");
    Serial.println(static_cast<uint8_t>(events[i].m_type));
    Serial.print("Value : ");
    Serial.println(events[i].m_data.value);
  }
}
