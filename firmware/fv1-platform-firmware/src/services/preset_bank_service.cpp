#include "services/preset_bank_service.h"

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

  EventBus::publish(e);
}

void PresetBankService::init() {
  loadPresetBank(m_logicalState.m_currentPresetBank, m_logicalState.m_loadedPresetBank);
}

void PresetBankService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kPresetBank
        && t_event.m_action == EventAction::kValueChanged) {
      if (t_event.m_data.value >= 0 && t_event.m_data.value < PresetConstants::c_presetBankCount) {
        loadPresetBank(t_event.m_data.value, m_logicalState.m_loadedPresetBank);
        publishSavePresetBankEvent(t_event);
      }
    }
  }

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kPresetBank
        && t_event.m_action == EventAction::kValueChanged) {
      uint8_t bank = Utils::wrappedAdd(m_logicalState.m_currentPresetBank, t_event.m_data.delta, PresetConstants::c_presetBankCount);

      loadPresetBank(bank, m_logicalState.m_loadedPresetBank);
      publishSavePresetBankEvent(t_event);
    }
  }
}

void PresetBankService::update() {

}

bool PresetBankService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kPresetBank
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kPresetBank
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  return false;
}
