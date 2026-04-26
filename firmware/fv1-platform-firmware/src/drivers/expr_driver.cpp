#include "drivers/expr_driver.h"

namespace hal {

  void ExprDriver::init() {
    m_detectPin.init();
    m_pot.init();

    m_state = static_cast<ExprDriverState>(m_detectPin.read());
  }

  size_t ExprDriver::poll(Event* t_outEvents, size_t t_maxEvents) {
    size_t eventCount = 0;
    bool switchEvent = !m_detectPin.read();

    uint32_t now = millis();

  }

}
