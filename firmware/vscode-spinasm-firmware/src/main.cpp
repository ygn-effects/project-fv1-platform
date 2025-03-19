#include <Arduino.h>
#include "hardware.h"

Hardware hardware;

void setup() {
  hardware.setup();
}

void loop() {
  hardware.process();
}
