#pragma once

#include <Arduino.h>

enum class AppState : uint8_t {
  kBoot,
  kRestoreState,
  kProgramIdle,
  kProgramEdit,
  kPresetIdle
};

class FSM {
  private:
    AppState m_state{AppState::kBoot};

  public:
    AppState getState() const;
    void setState(AppState t_state);
};
