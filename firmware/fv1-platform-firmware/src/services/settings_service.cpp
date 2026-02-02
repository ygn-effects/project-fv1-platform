#include "services/settings_service.h"

void SettingsService::saveRegion(MemoryRegion t_region, uint8_t t_programIndex, uint8_t t_potIndex) {
  RegionInfo info = m_handler.calculateRegionInfo(t_region, t_programIndex, t_potIndex);

  uint8_t buffer[info.m_length];
  m_handler.serializeRegion(t_region, m_logicalState, buffer, t_programIndex, t_potIndex);
  m_eeprom.write(info.m_address, buffer, info.m_length);
}

void SettingsService::loadRegion(MemoryRegion t_region, uint8_t t_programIndex, uint8_t t_potIndex) {
  RegionInfo info = m_handler.calculateRegionInfo(t_region, t_programIndex, t_potIndex);

  uint8_t buffer[info.m_length];
  m_eeprom.read(info.m_address, buffer, info.m_length);
  m_handler.deserializeRegion(t_region, m_logicalState, buffer, t_programIndex, t_potIndex);
}

void SettingsService::init() {
  m_eeprom.init();

  loadRegion(MemoryRegion::kLogicalState);
}

void SettingsService::handleEvent(const Event& t_event) {
  if (t_event.m_action == EventAction::kSave) {
    switch (t_event.m_subject) {
      case EventSubject::kBypass:
        saveRegion(MemoryRegion::kBypass);
        break;

      case EventSubject::kProgramMode:
        saveRegion(MemoryRegion::kProgramMode);
        break;

      case EventSubject::kProgram:
        saveRegion(MemoryRegion::kCurrentProgram);
        break;

      case EventSubject::kTap:
        saveRegion(MemoryRegion::kTap);
        break;

      case EventSubject::kTempo:
        saveRegion(MemoryRegion::kTempo);
        break;

      case EventSubject::kExpr:
        saveRegion(MemoryRegion::kExpr, m_logicalState.m_currentProgram);
        break;

      case EventSubject::kPot:
        saveRegion(MemoryRegion::kPot, m_logicalState.m_currentProgram, t_event.m_id);
        break;

      case EventSubject::kGeneral:
        saveRegion(MemoryRegion::kLogicalState);
        break;

      default:
        break;
    }
  }

  if (t_event.m_action == EventAction::kLoad) {
    switch (t_event.m_subject) {
      case EventSubject::kGeneral:
        loadRegion(MemoryRegion::kLogicalState);
        break;

      default:
        break;
    }
  }
}

void SettingsService::update() {

}

bool SettingsService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kMemory) {
    if (t_event.m_subject != EventSubject::kPreset
        && t_event.m_subject != EventSubject::kPresetBank) {
      return true;
    }
  }

  return false;
}
