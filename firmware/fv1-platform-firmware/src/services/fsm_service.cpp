#include "services/fsm_service.h"

void FsmService::transitionTo(AppState t_state, uint32_t t_timestamp) {
  m_state = t_state;

  Event e;
  e.m_type = EventType::kStateChanged;
  e.m_timestamp = t_timestamp; /*millis*/
  e.m_data.value = static_cast<uint16_t>(t_state);
  EventBus::publish(e);
}

void FsmService::init() {
  transitionTo(AppState::kRestoreState, /*Timestamp*/ 0);
}

void FsmService::handleEvent(const Event& t_event) {
  switch(m_state) {
    case AppState::kBoot:
    case AppState::kRestoreState:
      if (t_event.m_type == EventType::kBootCompleted) {
        if (m_logicalState.m_bypassState == BypassState::kActive) {
          transitionTo(m_logicalState.m_programMode == ProgramMode::kProgram
                        ? AppState::kProgramIdle
                        : AppState::kPresetIdle, t_event.m_timestamp);
        }
        else {
          transitionTo(AppState::kBypassed, t_event.m_timestamp);
        }
      }

      break;

    case AppState::kBypassed:
      // Bypass
      if (t_event.m_type == EventType::kRawBypassPressed) {
        EventBus::publish({EventType::kBypassPressed, t_event.m_timestamp, {}});
        return;
      }

      if (t_event.m_type == EventType::kBypassEnabled) {
        transitionTo(m_logicalState.m_programMode == ProgramMode::kProgram
                      ? AppState::kProgramIdle
                      : AppState::kPresetIdle, t_event.m_timestamp);
      }

      break;

    case AppState::kProgramIdle:
      // Bypass
      if (t_event.m_type == EventType::kRawBypassPressed) {
        EventBus::publish({EventType::kBypassPressed, t_event.m_timestamp, {}});
        return;
      }

      if (t_event.m_type == EventType::kBypassDisabled) {
        transitionTo(AppState::kBypassed, t_event.m_timestamp);
        return;
      }

      // Tap
      if (t_event.m_type == EventType::kRawTapPressed) {
        EventBus::publish({EventType::kTapPressed, t_event.m_timestamp, {}});
        return;
      }

      // Long‑tap
      if (t_event.m_type == EventType::kRawTapLongPressed) {
        EventBus::publish({EventType::kTapLongPressed, t_event.m_timestamp, {}});
        return;
      }

      // Encoder‑switch long press
      if (t_event.m_type == EventType::kRawMenuLockLongPressed) {
        EventBus::publish({EventType::kMenuLockLongPressed, t_event.m_timestamp, {}});
        return;
      }

      if (t_event.m_type == EventType::kMenuUnlocked) {
        transitionTo(AppState::kProgramEdit, t_event.m_timestamp);
        return;
      }

      if (t_event.m_type == EventType::kRawProgramModeSwitchLongPress) {
        transitionTo(AppState::kPresetIdle, t_event.m_timestamp);
        EventBus::publish({EventType::kProgramModeChanged, t_event.m_timestamp /*millis*/, {}});
        return;
      }

      break;

    case AppState::kProgramEdit:

      // Bypass
      if (t_event.m_type == EventType::kRawBypassPressed) {
        EventBus::publish({EventType::kBypassPressed, t_event.m_timestamp, {}});
        return;
      }

      if (t_event.m_type == EventType::kMenuLocked) {
        transitionTo(AppState::kProgramIdle, t_event.m_timestamp);
        return;
      }

      if (t_event.m_type == EventType::kRawMenuEncoderPressed) {
        EventBus::publish({EventType::kMenuEncoderPressed, t_event.m_timestamp, {}});
        return;
      }

      if (t_event.m_type == EventType::kRawMenuLockLongPressed) {
        EventBus::publish({EventType::kMenuLockLongPressed, t_event.m_timestamp, {}});
        return;
      }

      if (t_event.m_type == EventType::kRawMenuEncoderMoved) {
        Event e;
        e.m_type = EventType::kMenuEncoderMoved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.delta = t_event.m_data.delta;
        EventBus::publish(e);
      }

      // Tap
      if (t_event.m_type == EventType::kRawTapPressed) {
        EventBus::publish({EventType::kTapPressed, t_event.m_timestamp, {}});
        return;
      }

      // Long‑tap
      if (t_event.m_type == EventType::kRawTapLongPressed) {
        EventBus::publish({EventType::kTapLongPressed, t_event.m_timestamp, {}});
        return;
      }

      // Pot 0 move
      if (t_event.m_type == EventType::kRawPot0Moved && m_logicalState.m_bypassState==BypassState::kActive) {
        Event e;
        e.m_type = EventType::kPot0Moved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.value = t_event.m_data.value;
        EventBus::publish(e);
        return;
      }

      // Pot 1 move
      if (t_event.m_type == EventType::kRawPot1Moved && m_logicalState.m_bypassState==BypassState::kActive) {
        Event e;
        e.m_type = EventType::kPot1Moved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.value = t_event.m_data.value;
        EventBus::publish(e);
        return;
      }

      // Pot 2 move
      if (t_event.m_type == EventType::kRawPot2Moved && m_logicalState.m_bypassState==BypassState::kActive) {
        Event e;
        e.m_type = EventType::kPot2Moved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.value = t_event.m_data.value;
        EventBus::publish(e);
        return;
      }

      // Mix pot move
      if (t_event.m_type == EventType::kRawMixPotMoved && m_logicalState.m_bypassState==BypassState::kActive) {
        Event e;
        e.m_type = EventType::kMixPotMoved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.value = t_event.m_data.value;
        EventBus::publish(e);
        return;
      }

      if (t_event.m_type == EventType::kMenuLocked) {
        transitionTo(AppState::kProgramIdle, t_event.m_timestamp);
        return;
      }

      if (t_event.m_type == EventType::kBypassDisabled) {
        transitionTo(AppState::kProgramIdle, t_event.m_timestamp);
      }

      break;

    case AppState::kPresetIdle:
      // Bypass
      if (t_event.m_type == EventType::kRawBypassPressed) {
        EventBus::publish({EventType::kBypassPressed, t_event.m_timestamp, {}});
        return;
      }

      if (t_event.m_type == EventType::kRawProgramModeSwitchLongPress) {
        transitionTo(AppState::kProgramIdle, t_event.m_timestamp);
        EventBus::publish({EventType::kProgramModeChanged, t_event.m_timestamp /*millis*/, {}});
        return;
      }

    default:
      break;
  }
}

void FsmService::update() {

}

bool FsmService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kRawPhysicalEvent || t_category == EventCategory::kBootEvent
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuLockEvent);
}
