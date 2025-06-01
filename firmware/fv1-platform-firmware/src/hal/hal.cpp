#include "hal/hal.h"

namespace hal {

ArduinoClock clock;

DigitalGpioDriver eepromCsPin(0, GpioConfig::kOutput);
M95Driver eeprom(eepromCsPin);

DigitalGpioDriver bypassRelayPin(23, GpioConfig::kOutput);
DigitalGpioDriver bypassOptoCouplerPin(22, GpioConfig::kOutput);
DigitalGpioDriver bypassLedPin(12, GpioConfig::kOutput);
Bypass bypass(bypassRelayPin, bypassOptoCouplerPin, bypassLedPin);

SSD1306Driver display;

Pcf8574Expander expander(0x20);

ExpanderGpioDriver menuEncoderPinA(1, GpioConfig::kInputPullup, expander);
ExpanderGpioDriver menuEncoderPinB(2, GpioConfig::kInputPullup, expander);
EncoderDriver menuEncoder(menuEncoderPinA, menuEncoderPinB, EncoderId::kMenuEncoder);

ExpanderGpioDriver menuEncoderSwitchPin(4, GpioConfig::kInputPullup, expander);
SwitchDriver menuEncoderSwitch(menuEncoderSwitchPin, SwitchId::kMenuEncoder);

ExpanderGpioDriver programModeSwitchPin(3, GpioConfig::kInputPullup, expander);
SwitchDriver programModeSwitch(programModeSwitchPin, SwitchId::kProgramMode);

ExpanderGpioDriver programModeLedPin(0, GpioConfig::kOutput, expander);
LedDriver programModeLed(programModeLedPin);

ExpanderGpioDriver menuLockSwitchPin(5, GpioConfig::kInputPullup, expander);
SwitchDriver menuLockSwitch(menuLockSwitchPin, SwitchId::kMenuLock);

ExpanderGpioDriver menuLockLedPin(6, GpioConfig::kOutput, expander);
LedDriver menuLockLed(menuLockLedPin);

DigitalGpioDriver bypassSwitchPin(1, GpioConfig::kInputPullup);
SwitchDriver bypassSwitch(bypassSwitchPin, SwitchId::kBypass);

DigitalGpioDriver tapSwitchPin(3, GpioConfig::kInputPullup);
SwitchDriver tapSwitch(tapSwitchPin, SwitchId::kTap);

AnalogGpioDriver tapSwitchLedPin(15, GpioConfig::kOutput);
AdjustableLedDriver tapSwitchLed(tapSwitchLedPin);

AnalogGpioDriver pot0Pin(24, GpioConfig::kInput);
PotDriver pot0(pot0Pin, PotId::kPot0);

AnalogGpioDriver pot1Pin(25, GpioConfig::kInput);
PotDriver pot1(pot1Pin, PotId::kPot1);

AnalogGpioDriver pot2Pin(26, GpioConfig::kInput);
PotDriver pot2(pot2Pin, PotId::kPot2);

AnalogGpioDriver mixPotPin(27, GpioConfig::kInput);
PotDriver mixPot(mixPotPin, PotId::kMixPot);

PollManager pollManager;

void init() {
  pollManager.registerDevice(&expander);
  pollManager.registerDevice(&menuEncoder);
  pollManager.registerDevice(&menuEncoderSwitch);
  pollManager.registerDevice(&programModeSwitch);
  pollManager.registerDevice(&menuLockSwitch);
  pollManager.registerDevice(&bypassSwitch);
  pollManager.registerDevice(&tapSwitch);
  pollManager.registerDevice(&pot0);
  pollManager.registerDevice(&pot1);
  pollManager.registerDevice(&pot2);
  pollManager.registerDevice(&mixPot);

  pollManager.init();
  delay(100);
  pollManager.poll();
  delay(100);
  EventBus::clear();
}

void update() {
  pollManager.poll();
}

}
