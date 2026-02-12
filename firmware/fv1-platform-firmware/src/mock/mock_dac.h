#include "periphs/dac.h"

class MockDac : public Dac {
  public:
    bool m_initialized = false;
    uint16_t m_lastValue = 0;
    uint16_t m_writeCount = 0;

    void init() override {
      m_initialized = true;
    }

    void write(uint16_t t_value) override {
      m_lastValue = t_value;
      m_writeCount++;
    }
};
