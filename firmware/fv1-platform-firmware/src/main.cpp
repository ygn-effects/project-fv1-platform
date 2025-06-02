#include <Arduino.h>
#include "hal/hal.h"
#include "services/services.h"

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("Setup...");
  hal::init();
  services::init();

  EventBus::publish({EventType::kBootCompleted, 0, {}});

  Serial.println("Setup done, starting main loop...");
}

void loop() {
  hal::update();
  services::update();
}
