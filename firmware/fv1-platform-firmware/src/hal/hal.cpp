#include "hal/hal.h"

namespace hal {

ArduinoClock clock;

DigitalGpioDriver eepromCsPin(4, GpioConfig::kOutput);
M95Driver eeprom(eepromCsPin);

DigitalGpioDriver bypassRelayPin(8, GpioConfig::kOutput);
DigitalGpioDriver bypassOptoCouplerPin(11, GpioConfig::kOutput);
DigitalGpioDriver bypassSwitchLedPin(21, GpioConfig::kOutput);
Bypass bypass(bypassRelayPin, bypassOptoCouplerPin, bypassSwitchLedPin);

SerialDriver midiSerial(31250);

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

DigitalGpioDriver bypassSwitchPin(22, GpioConfig::kInputPullup);
SwitchDriver bypassSwitch(bypassSwitchPin, SwitchId::kBypass);

DigitalGpioDriver tapSwitchPin(20, GpioConfig::kInputPullup);
SwitchDriver tapSwitch(tapSwitchPin, SwitchId::kTap);

AnalogGpioDriver tapSwitchLedPin(15, GpioConfig::kOutput);
AdjustableLedDriver tapSwitchLed(tapSwitchLedPin);

AnalogGpioDriver pot0Pin(28, GpioConfig::kInput);
PotDriver pot0(pot0Pin, PotId::kPot0);

AnalogGpioDriver pot1Pin(27, GpioConfig::kInput);
PotDriver pot1(pot1Pin, PotId::kPot1);

AnalogGpioDriver pot2Pin(25, GpioConfig::kInput);
PotDriver pot2(pot2Pin, PotId::kPot2);

AnalogGpioDriver mixPotPin(26, GpioConfig::kInput);
PotDriver mixPot(mixPotPin, PotId::kMixPot);

DigitalGpioDriver fv1S0Pin(13, GpioConfig::kOutput);
DigitalGpioDriver fv1S1Pin(0, GpioConfig::kOutput);
DigitalGpioDriver fv1S2Pin(1, GpioConfig::kOutput);
DigitalGpioDriver fv1Dac1CsPin(2, GpioConfig::kOutput);
DigitalGpioDriver fv1Dac2CsPin(14, GpioConfig::kOutput);
Mcp4912 fv1P0Dac(fv1Dac1CsPin, Channel::kB);
Mcp4912 fv1P1Dac(fv1Dac1CsPin, Channel::kA);
Mcp4911 fv1P2Dac(fv1Dac2CsPin);
Fv1Driver fv1(fv1S0Pin, fv1S1Pin, fv1S2Pin, fv1P0Dac, fv1P1Dac, fv1P2Dac);

DigitalGpioDriver vcaDac3CsPin(3, GpioConfig::kOutput);
Mcp4912 vcaWetDac(vcaDac3CsPin, Channel::kA);
Mcp4912 vcaDryDac(vcaDac3CsPin, Channel::kB);

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
