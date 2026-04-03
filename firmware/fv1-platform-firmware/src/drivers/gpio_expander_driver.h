#include "drivers/gpio_driver.h"
#include "periphs/gpio.h"
#include "periphs/gpio_expander.h"

namespace hal {

class ExpanderGpioDriver : public DigitalGpio {
  private:
    uint8_t m_pin;
    GpioConfig m_config;
    GpioExpander& m_expander;

  public:
    explicit ExpanderGpioDriver (uint8_t t_pin, GpioConfig t_config, GpioExpander& t_expander)
      : m_pin(t_pin), m_config(t_config), m_expander(t_expander) {}

    void init() override {
      switch (m_config) {
        case GpioConfig::kInput:
        case GpioConfig::kInputPullup:
          m_expander.writePin(m_pin, 1);
          break;

        case GpioConfig::kOutput:
          m_expander.writePin(m_pin, 0);
          break;

        default:
          break;
      }
    }

    bool read() override {
      return m_expander.readPin(m_pin);
    }

    void write(bool t_value) override {
      m_expander.writePin(m_pin, t_value);
    }
};

}
