#include "services/preset_service.h"

void PresetService::applyPreset() {
  m_presetHandler.applyToState(m_logicalState, m_logicalState.m_currentPreset);
}

void PresetService::savePreset(uint8_t t_bankIndex, uint8_t t_presetIndex) {
  RegionInfo info = m_memoryHandler.calculateRegionInfo(MemoryRegion::kPreset, t_bankIndex, t_presetIndex);

  uint8_t buffer[info.m_length];
  m_memoryHandler.serializePreset(m_logicalState.m_loadedPresetBank.m_presets[t_presetIndex], buffer, t_bankIndex, t_presetIndex, 0);
  m_eeprom.write(info.m_address, buffer, info.m_length);
}

void PresetService::publishSavePresetEvent(const Event& t_event) {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSave;

  EventBus::publish(e);
}

void PresetService::init() {
  if (m_logicalState.m_programMode == ProgramMode::kPreset) {
    applyPreset();
  }
}

void PresetService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kValueChanged) {
      m_logicalState.m_currentPreset = Utils::wrappedAdd(m_logicalState.m_currentPreset, t_event.m_data.delta, PresetConstants::c_presetPerBank);
      applyPreset();
      publishSavePresetEvent(t_event);

      return;
    }
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kValueChanged) {
      if (t_event.m_data.value >= 0 && t_event.m_data.value < PresetConstants::c_presetPerBank) {
        m_logicalState.m_currentPreset = t_event.m_data.value;
        applyPreset();
        publishSavePresetEvent(t_event);

        return;
      }
    }
  }

  if (t_event.m_domain == EventDomain::kMemory
      && t_event.m_subject == EventSubject::kPresetBank
      && t_event.m_action == EventAction::kLoad) {
    m_logicalState.m_currentPreset = 0;
    applyPreset();
    publishSavePresetEvent(t_event);

    return;
  }

  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kTap
        && t_event.m_action == EventAction::kPressed) {
      m_logicalState.m_currentPreset = Utils::wrappedAdd(m_logicalState.m_currentPreset, 1, PresetConstants::c_presetPerBank);
      applyPreset();
      publishSavePresetEvent(t_event);

      return;
    }

    if (t_event.m_subject == EventSubject::kTap
        && t_event.m_action == EventAction::kLongPressed) {
      m_logicalState.m_currentPreset = Utils::wrappedAdd(m_logicalState.m_currentPreset, -1, PresetConstants::c_presetPerBank);
      applyPreset();
      publishSavePresetEvent(t_event);

      return;
    }
  }

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kSave) {
      // TBD
    }

    if (t_event.m_subject == EventSubject::kProgramMode
        && t_event.m_action == EventAction::kToggled) {
      init();
    }
  }
}

void PresetService::update() {

}

bool PresetService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kMemory) {
    if (t_event.m_subject == EventSubject::kPresetBank
        && t_event.m_action == EventAction::kLoad) return true;
  }

  // Physical events are only processed in preset mode
  if (m_logicalState.m_programMode == ProgramMode::kPreset) {
    if (t_event.m_domain == EventDomain::kPhysical) {
      if (t_event.m_subject == EventSubject::kTap
          && t_event.m_action == EventAction::kPressed
          || t_event.m_action == EventAction::kLongPressed) return true;
    }
  }

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kSave) return true;

    if (t_event.m_subject == EventSubject::kProgramMode
        && t_event.m_action == EventAction::kToggled) return true;
  }

  return false;
}
