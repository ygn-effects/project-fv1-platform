#include <Arduino.h>
#include "peripherals/toggle_device.h"

#ifndef BYPASS_H
#define BYPASS_H

/**
 * @brief Drives a relay/optocoupler pair to actually bypass or not the effect and act as a fake "on/off" switch.
 */
class Bypass {
  private:
    ToggleDevice m_relay; // Relay
    ToggleDevice m_optocoupler; // Optocoupler

  public:
    /**
     * @brief Construct a new Bypass object
     *
     * @param t_relPin Relay pin #
     * @param t_okPin Optical relay pin #
     */
    Bypass(uint8_t t_relPin, uint8_t t_okPin) : m_relay(t_relPin), m_optocoupler(t_okPin) { };

    /**
     * @brief Setup the pins
     */
    void setup();

    /**
     * @brief set the bypass state
     */
    void setState(uint8_t t_state);

    /**
     * @brief switch the bypass state
     */
    void switchState();
};

#endif