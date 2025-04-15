#include "fsm.h"

AppState FSM::getState() const {
  return m_state;
}

void FSM::setState(AppState t_state) {
  m_state = t_state;
}
