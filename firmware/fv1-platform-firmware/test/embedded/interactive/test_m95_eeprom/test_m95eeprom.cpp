#include "hal/poll_manager.h"
#include "drivers/eeprom_m95.h"

hal::DigitalGpioDriver eepromCsPin(0, hal::GpioConfig::kOutput);
hal::M95Driver eeprom(eepromCsPin);

void setup() {
  Serial.begin(31250);
  delay(2000);

  Serial.println("EEPROM test starting...");

  eeprom.init();

  uint8_t input[1];
  input[0] = 4;
  eeprom.write(150, input, 1);

  uint8_t output[1];
  output[0] = 3;
  eeprom.read(150, output, 1);

  Serial.println(output[0]);

  uint8_t input2[100];
  for (uint8_t i = 0; i < 100; i++) {
    input2[i] = i;
  }

  eeprom.write(0, input2, 100);

  uint8_t output2[100];
  eeprom.read(0, output2, 100);

  for (uint8_t i = 0; i < 100; i++) {
    Serial.println(output2[i]);
  }


  Serial.println("Setup done main loop starting...");
}

void loop() {

}
