#include <Arduino.h>
#include "hal/hal.h"

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("Setup...");
  hal::init();

  Serial.println("Setup done, starting main loop...");
}

void loop() {
  hal::update();

  while (EventBus::hasEvent()) {
    Event e;
    EventBus::recall(e);

    Serial.print("Received event : ");
    Serial.println(static_cast<uint8_t>(e.m_type));

    if (e.m_data.value) {
      Serial.print("Value : ");
      Serial.println(e.m_data.value);
    }

    if (e.m_data.delta) {
      Serial.print("Delta : ");
      Serial.println(e.m_data.delta);
    }
  }
}
