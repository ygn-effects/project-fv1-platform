#include "hal/poll_manager.h"
#include "drivers/encoder.h"

hal::EncoderDriver testEncoder(2, 3, EncoderId::kMenuEncoder);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("Encoder test starting...");
  testEncoder.init();

  Serial.println("Setup done main loop starting...");
}

void loop() {
  Event events[4];
  size_t eventsCount = 0;

  eventsCount = testEncoder.poll(events, 4);

  for (uint8_t i = 0; i < eventsCount; i++) {
    Serial.print("Received event : ");
    Serial.println(static_cast<uint8_t>(events[i].m_type));
    Serial.print("Delta : ");
    Serial.println(events[i].m_data.delta);
  }
}
