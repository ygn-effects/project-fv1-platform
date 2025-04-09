#pragma once

#include <Arduino.h>

/// @brief Drives an LED, it can be turned on or off,
/// have its state toggled and blink
class Led {
  protected:
    uint8_t m_pin;
    uint8_t m_ledState;
    uint32_t m_lastBlinkTime = 0;
    uint8_t m_lastBlinkState = 0;

  public:
    /// @brief Construct an LED object
    /// @param t_pin Pin #
    /// @param t_initialState Initial state of the LED
    Led(uint8_t t_pin, uint8_t t_initialState = LOW) :
      m_pin(t_pin),
      m_ledState(t_initialState) { };

    /// @brief Initialize the LED pin as an output and set the initial state
    void setup();

    /// @brief Turn the LED on
    void turnOn();

    /// @brief Turn the LED off
    void turnOff();

    /// @brief Set the LED to a desired state
    /// @param state The new state of the LED (HIGH or LOW)
    void setState(uint8_t t_state);

    /// @brief Toggle the LED state between on/off
    void toggle();

    /// @brief Get the current state of the LED (HIGH/LOW)
    /// @return uint8_t 1 : ON / 0 : OFF
    uint8_t getState() const;

    /// @brief Blink the LED at a specified interval
    /// @param interval The ON/OFF interval in milliseconds
    void blink(uint8_t t_interval);
};

/// @brief Drives a PWM-controlled LED with variable brightness
class PwmLed : public Led {
  public:
    /// @brief Construct a new Pwm Led object
    /// @param pin pin #
    PwmLed(uint8_t t_pin) :
      Led(t_pin) { };

    /// @brief Set the brightness of the PWM LED
    /// @param brightness Brightness level (0-255)
    void setBrightness(uint8_t brightness);
};

