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

SettingsService::SettingsService(LogicalState& t_lState, EEPROM& t_eeprom)
  : m_logicalState(t_lState),
    m_eeprom(t_eeprom) {}

void SettingsService::init() {
  m_eeprom.init();

  loadRegion(MemoryRegion::kLogicalState);
}

void SettingsService::handleEvent(const Event& t_event) {
  switch (t_event.m_type) {
    case EventType::kSaveBypass:
      saveRegion(MemoryRegion::kBypass);
      break;

    case EventType::kSaveProgramMode:
      saveRegion(MemoryRegion::kProgramMode);
      break;

    case EventType::kSaveCurrentProgram:
      saveRegion(MemoryRegion::kCurrentProgram);
      break;

    case EventType::kSaveCurrentPreset:
      saveRegion(MemoryRegion::kCurrentPreset);
      break;

    case EventType::kSaveCurrentPresetBank:
      saveRegion(MemoryRegion::kCurrentPresetBank);
      break;

    case EventType::kSaveMidiChannel:
      saveRegion(MemoryRegion::kMidiChannel);
      break;

    case EventType::kSaveDeviceState:
      saveRegion(MemoryRegion::kDeviceState);
      break;

    case EventType::kSaveTap:
      saveRegion(MemoryRegion::kTap);
      break;

    case EventType::kSaveTempo:
      saveRegion(MemoryRegion::kTempo);
      break;

    case EventType::kSavePot:
      saveRegion(MemoryRegion::kPot, m_logicalState.m_currentProgram, t_event.m_data.value);
      break;

    case EventType::kSaveExpr:
      saveRegion(MemoryRegion::kExpr, m_logicalState.m_currentProgram);
      break;

    case EventType::kRawProgramModeSwitchLongPress:
    case EventType::kSaveLogicalState:
      saveRegion(MemoryRegion::kLogicalState);
      break;

    case EventType::kRestoreState:
      loadRegion(MemoryRegion::kLogicalState);
      break;

    default:
      break;
  }
}

void SettingsService::update() {

}

bool SettingsService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kSaveEvent
      || t_category == EventCategory::kLoadEvent
      || t_category == EventCategory::kBootEvent
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPresetBankChangedEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramModeChangedEvent);
}
