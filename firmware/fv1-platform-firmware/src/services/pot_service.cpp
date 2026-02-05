#include "services/pot_service.h"

void PotService::syncHandler() {
  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    m_handler.m_state[i] = m_logicalState.m_potParams[m_logicalState.m_currentProgram][i].m_state;
    m_handler.m_minValue[i] = m_logicalState.m_potParams[m_logicalState.m_currentProgram][i].m_minValue;
    m_handler.m_maxValue[i] = m_logicalState.m_potParams[m_logicalState.m_currentProgram][i].m_maxValue;
  }
}

void PotService::publishPotValueChangedEvent(uint8_t t_potIndex, uint32_t t_timestamp) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_timestamp = t_timestamp;
  e.m_id = t_potIndex;

  EventBus::publish(e);
}

void PotService::publishSavePotEvent(uint8_t t_potIndex) {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kSave;
  e.m_id = t_potIndex;
  e.m_timestamp = 0; // millis()

  EventBus::publish(e);
}

void PotService::publishTempoInputEvent(uint16_t t_value, uint32_t t_timestamp) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kInputChanged;
  e.m_timestamp = t_timestamp;
  e.m_data.value = t_value;

  EventBus::publish(e);
}

void PotService::handlePhysicalEvent(const Event& t_event) {
  uint16_t scaledValue = m_handler.mapAdcValue(t_event.m_data.value, t_event.m_id);

  // For delay effects, POT0 controls tempo
  if (static_cast<PotId>(t_event.m_id) == PotId::kPot0 && m_logicalState.m_activeProgram->m_isDelayEffect) {
    publishTempoInputEvent(scaledValue, t_event.m_timestamp);
    return;
  }

  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  if (params[t_event.m_id].m_state == PotState::kActive) {
    params[t_event.m_id].m_value = scaledValue;
    publishPotValueChangedEvent(t_event.m_id, t_event.m_timestamp);
  }
}

void PotService::handleMenuEvent(const Event& t_event) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  params[t_event.m_id].m_value = m_handler.mapMenuValue(params[t_event.m_id].m_value, t_event.m_data.delta, t_event.m_id);
  publishPotValueChangedEvent(t_event.m_id, t_event.m_timestamp);
}

void PotService::handleMidiEvent(const Event& t_event) {
  uint16_t scaledValue = m_handler.mapMidiValue(t_event.m_data.value, t_event.m_id);

  // For delay effects, POT0 controls tempo - scale MIDI 0-127 to 0-1023 and forward
  if (static_cast<PotId>(t_event.m_id) == PotId::kPot0 && m_logicalState.m_activeProgram->m_isDelayEffect) {
    publishTempoInputEvent(scaledValue, t_event.m_timestamp);
    return;
  }

  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  params[t_event.m_id].m_value = scaledValue;
  publishPotValueChangedEvent(t_event.m_id, t_event.m_timestamp);
}

void PotService::handleExprEvent(const Event& t_event) {
  // For delay effects, POT0 controls tempo - forward raw value to TempoService
  if (static_cast<PotId>(t_event.m_id) == PotId::kPot0 && m_logicalState.m_activeProgram->m_isDelayEffect) {
    publishTempoInputEvent(t_event.m_data.value, t_event.m_timestamp);
    return;
  }

  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  params[t_event.m_id].m_value = t_event.m_data.value;
  publishPotValueChangedEvent(t_event.m_id, t_event.m_timestamp);
}

void PotService::handleMenuPotStateToggleEvent(const Event& t_event, uint8_t t_potIndex) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  params[t_potIndex].m_state = m_handler.togglePotState(t_potIndex);

  publishSavePotEvent(t_potIndex);
}

void PotService::handleMenuPotMinValueMove(const Event& t_event, uint8_t t_potIndex) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  params[t_potIndex].m_minValue = m_handler.changePotMinValue(t_event.m_data.delta, t_potIndex);

  publishSavePotEvent(t_potIndex);
}

void PotService::handleMenuPotMaxValueMove(const Event& t_event, uint8_t t_potIndex) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  params[t_potIndex].m_maxValue = m_handler.changePotMaxValue(t_event.m_data.delta, t_potIndex);

  publishSavePotEvent(t_potIndex);
}

void PotService::copyPotValues(uint8_t t_targetProgram) {
  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    m_logicalState.m_potParams[t_targetProgram][i].m_value =
      m_logicalState.m_potParams[m_lastSyncedProgram][i].m_value;
  }
}

void PotService::init() {
  syncHandler();
  m_lastSyncedProgram = m_logicalState.m_currentProgram;
}

void PotService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) {
      handlePhysicalEvent(t_event);
      return;
    }
  }

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) {
      if (m_logicalState.m_programMode == ProgramMode::kProgram) {
        copyPotValues(m_logicalState.m_currentProgram);
      }

      m_lastSyncedProgram = m_logicalState.m_currentProgram;
      syncHandler();
      return;
    }

    if (t_event.m_subject == EventSubject::kExpr
        && t_event.m_action == EventAction::kValueChanged) {
      handleExprEvent(t_event);
      return;
    }
  }

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kSettingChanged) {
      uint8_t index = 0, param = 0;
      Utils::pack8(t_event.m_id, index, param);

      switch (static_cast<PotParam>(param)) {
        case PotParam::kState:
          handleMenuPotStateToggleEvent(t_event, index);
          break;

        case PotParam::kMinValue:
          handleMenuPotMinValueMove(t_event, index);
          break;

        case PotParam::kMaxValue:
          handleMenuPotMaxValueMove(t_event, index);
          break;

        default:
          break;
      }

      return;
    }

    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) {
      handleMenuEvent(t_event);
      return;
    }
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) {
      handleMidiEvent(t_event);
      return;
    }
  }
}

void PotService::update() {

}

bool PotService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) return true;

    if (t_event.m_subject == EventSubject::kExpr
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kPot) return true;
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kPot) return true;
  }

  return false;
}
