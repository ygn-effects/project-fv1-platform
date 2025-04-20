#include "services/fsm_service.h"

void FsmService::transitionTo(AppState t_state, uint32_t t_timestamp) {
  m_state = t_state;

  Event e{ EventType::kStateChanged, t_timestamp, {.value = static_cast<uint16_t>(t_state)} };
  EventBus::publish(e);
}

void FsmService::init() {
  transitionTo(AppState::kRestoreState, /*Timestamp*/ 0);
}

void FsmService::handleEvent(const Event& t_event) {
  switch(m_state) {
    case AppState::kBoot:
      if (t_event.m_type == EventType::kBootCompleted) {
        transitionTo(m_logicalState.m_programMode == ProgramMode::kProgram
                      ? AppState::kProgramIdle
                      : AppState::kPresetIdle, t_event.m_timestamp);
      }

    case AppState::kProgramIdle:
      // Bypass
      if (t_event.m_type == EventType::kRawBypassPressed) {
        EventBus::publish({EventType::kBypassPressed, t_event.m_timestamp, {}});
        return;
      }

      // Tap
      if (t_event.m_type == EventType::kRawTapPressed && m_logicalState.m_bypassState==BypassState::kActive) {
        EventBus::publish({EventType::kTapPressed, t_event.m_timestamp, {}});
        return;
      }

      // Long‑tap
      if (t_event.m_type == EventType::kRawTapLongPressed && m_logicalState.m_bypassState==BypassState::kActive) {
        EventBus::publish({EventType::kTapLongPressed, t_event.m_timestamp, {}});
        return;
      }

      // 4) Encoder‑switch press -> enter edit mode
      if (t_event.m_type == EventType::kRawMenuEncoderPressed && m_logicalState.m_bypassState==BypassState::kActive) {
        transitionTo(AppState::kProgramEdit, t_event.m_timestamp);
        return;
      }

      break;

    case AppState::kProgramEdit:
    // 4) Encoder‑switch press -> exit edit mode
      if (t_event.m_type == EventType::kRawMenuEncoderLongPressed) {
        transitionTo(AppState::kProgramIdle, t_event.m_timestamp);
        return;
      }

      break;

    case AppState::kPresetIdle:

    default:
      break;
  }
}

void FsmService::update() {

}
