#include "services/preset_bank_service.h"

void PresetBankService::syncSavePresetState() {
  m_logicalState.m_saveTargetBank = m_logicalState.m_currentPresetBank;
}

void PresetBankService::loadPresetBank(uint8_t t_bankIndex, PresetBank& t_presetBank) {
  RegionInfo info = m_handler.calculateRegionInfo(MemoryRegion::kPresetBank, t_bankIndex);

  uint8_t buffer[info.m_length];
  m_eeprom.read(info.m_address, buffer, info.m_length);
  m_handler.deserializePresetBank(t_presetBank, buffer, t_bankIndex, 0);

  m_logicalState.m_currentPresetBank = t_bankIndex;
}

void PresetBankService::publishSavePresetBankEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kSave;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void PresetBankService::publishPresetBankValueChangedEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void PresetBankService::init() {
  syncSavePresetState();
  loadPresetBank(m_logicalState.m_currentPresetBank, m_logicalState.m_loadedPresetBank);
}

void PresetBankService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kPresetBank
        && t_event.m_action == EventAction::kValueChanged) {
      uint8_t bank = Utils::wrappedAdd(m_logicalState.m_currentPresetBank, t_event.m_data.delta, PresetConstants::c_presetBankCount);

      loadPresetBank(bank, m_logicalState.m_loadedPresetBank);
      syncSavePresetState();
      publishPresetBankValueChangedEvent(t_event);
      publishSavePresetBankEvent(t_event);

      return;
    }

    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kSave) {
      if (m_logicalState.m_saveTargetBank < PresetConstants::c_presetBankCount
          && m_logicalState.m_saveTargetBank != m_logicalState.m_currentPresetBank) {
        loadPresetBank(m_logicalState.m_saveTargetBank, m_logicalState.m_loadedPresetBank);
        publishSavePresetBankEvent(t_event);

        return;
      }
    }

    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kSettingChanged) {
      if (static_cast<SavePresetParam>(t_event.m_id) == SavePresetParam::kTargetBank) {
        m_logicalState.m_saveTargetBank = Utils::clampedAdd(m_logicalState.m_saveTargetBank, t_event.m_data.delta, PresetConstants::c_presetBankCount - 1);
      }
    }
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kValueChanged) {
      uint8_t targetBank = t_event.m_data.value / PresetConstants::c_presetPerBank;

      if (targetBank >= PresetConstants::c_presetBankCount) return;

      if (targetBank != m_logicalState.m_currentPresetBank) {
        loadPresetBank(targetBank, m_logicalState.m_loadedPresetBank);
        syncSavePresetState();
        publishSavePresetBankEvent(t_event);

        return;
      }
    }
  }
}

void PresetBankService::update() {

}

bool PresetBankService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kPresetBank
        && t_event.m_action == EventAction::kValueChanged) return true;

    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kSave) return true;

    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kSettingChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  return false;
}
