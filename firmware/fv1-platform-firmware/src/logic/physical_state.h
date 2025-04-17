#pragma once

#include "peripherals/bypass.h"
#include "peripherals/switch.h"
#include "peripherals/encoder.h"
#include "peripherals/potentiometer.h"
#include "peripherals/led.h"

class PhysicalState {
  public:
    MomentarySwitch m_bypassSwitch;
    MomentarySwitch m_tapSwitch;
    MomentarySwitch m_menuEncoderSwitch;

    // encoder
    Encoder m_menuEncoder;

    // analog controls
    Potentiometer m_pot0;
    Potentiometer m_pot1;
    Potentiometer m_pot2;
    Potentiometer m_mixPot;

    // LEDs
    Led m_bypassLed;
    PwmLed m_tapLed;

    // Construct with pin assignments
    PhysicalState(uint8_t pinBypSw, uint8_t pinTapSw,
                  uint8_t pinEncA, uint8_t pinEncB, uint8_t pinEncSw,
                  uint8_t pinPot0, uint8_t pinPot1, uint8_t pinPot2, uint8_t pinMix,
                  uint8_t pinBypLed, uint8_t pinTapLed)
      : m_bypassSwitch(pinBypSw), m_tapSwitch(pinTapSw), m_menuEncoderSwitch(pinEncSw),
        m_menuEncoder(pinEncA, pinEncB),
        m_pot0(pinPot0), m_pot1(pinPot1), m_pot2(pinPot2), m_mixPot(pinMix),
        m_bypassLed(pinBypLed), m_tapLed(pinTapLed) {}

    void setup() {
        m_bypassSwitch.setup();
        m_tapSwitch.setup();
        m_menuEncoder.setup();
        m_menuEncoderSwitch.setup();
        m_pot0.setup(); m_pot1.setup(); m_pot2.setup(); m_mixPot.setup();
        m_bypassLed.setup(); m_tapLed.setup();
    }
};