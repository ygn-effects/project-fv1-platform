#include "hal/hal.h"

namespace hal {

Pcf8574Expander expander(0x20);

ExpanderGpioDriver menuEncoderPinA(1, GpioConfig::kInputPullup, expander);
ExpanderGpioDriver menuEncoderPinB(2, GpioConfig::kInputPullup, expander);
EncoderDriver menuEncoder(menuEncoderPinA, menuEncoderPinB, EncoderId::kMenuEncoder);

ExpanderGpioDriver menuEncoderSwitchPin(3, GpioConfig::kInputPullup, expander);
SwitchDriver menuEncoderSwitch(menuEncoderSwitchPin, SwitchId::kMenuEncoder);

ExpanderGpioDriver programModeSwitchPin(4, GpioConfig::kInputPullup, expander);
SwitchDriver programModeSwitch(programModeSwitchPin, SwitchId::kProgramMode);

ExpanderGpioDriver programModeLedPin(0, GpioConfig::kOutput, expander);
LedDriver programModeLed(programModeLedPin);

DigitalGpioDriver bypassSwitchPin(1, GpioConfig::kInputPullup);
SwitchDriver bypassSwitch(bypassSwitchPin, SwitchId::kBypass);

DigitalGpioDriver bypassSwitchLedPin(2, GpioConfig::kOutput);
LedDriver bypassSwitchLed(bypassSwitchLedPin);

DigitalGpioDriver tapSwitchPin(3, GpioConfig::kInputPullup);
SwitchDriver tapSwitch(tapSwitchPin, SwitchId::kTap);

AnalogGpioDriver tapSwitchLedPin(12, GpioConfig::kOutput);
BreathingLedDriver tapSwithLed(tapSwitchLedPin, 0);

PollManager pollManager;

void init() {
  expander.init();
  pollManager.registerDevice(&menuEncoder);
  pollManager.registerDevice(&menuEncoderSwitch);
  pollManager.registerDevice(&programModeSwitch);
  pollManager.registerDevice(&bypassSwitch);
  pollManager.registerDevice(&tapSwitch);

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
