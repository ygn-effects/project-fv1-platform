#include <Arduino.h>
#include "hal/hal.h"
#include "services/services.h"

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("Setup...");
  hal::init();
  services::init();

  Event e;
  e.m_domain = EventDomain::kSystem;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kBooted;
  EventBus::publish(e);

  Serial.println("Setup done, starting main loop...");
}

void loop() {
  hal::update();
  services::update();
}
