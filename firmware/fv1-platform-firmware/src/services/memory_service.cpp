#include "services/memory_service.h"

void MemoryService::saveRegion(MemoryRegion t_region, uint8_t t_programIndex, uint8_t t_potIndex) {
  RegionInfo info = m_handler.calculateRegionInfo(t_region, t_programIndex, t_potIndex);

  uint8_t buffer[info.m_length];
  m_handler.serializeRegion(t_region, m_logicalState, buffer, t_programIndex, t_potIndex);
  m_eeprom->write(info.m_address, buffer, info.m_length);
}

void MemoryService::loadRegion(MemoryRegion t_region, uint8_t t_programIndex, uint8_t t_potIndex) {
  RegionInfo info = m_handler.calculateRegionInfo(t_region, t_programIndex, t_potIndex);

  uint8_t buffer[info.m_length];
  m_eeprom->read(info.m_address, buffer, info.m_length);
  m_handler.deserializeRegion(t_region, m_logicalState, buffer, t_programIndex, t_potIndex);
}

void MemoryService::loadPresetBank(uint8_t t_bankIndex) {
  RegionInfo info = m_handler.calculateRegionInfo(MemoryRegion::kPresetBank, t_bankIndex);

  uint8_t buffer[info.m_length];
  m_eeprom->read(info.m_address, buffer, info.m_length);
  m_handler.deserializePresetBank(m_presetBank, buffer, t_bankIndex, 0);
}

void MemoryService::savePreset(uint8_t t_bankIndex, uint8_t t_presetIndex) {
  RegionInfo info = m_handler.calculateRegionInfo(MemoryRegion::kPreset, t_bankIndex, t_presetIndex);

  uint8_t buffer[info.m_length];
  m_handler.serializePreset(m_presetBank.m_presets[t_presetIndex], buffer, t_bankIndex, t_presetIndex, 0);
  m_eeprom->write(info.m_address, buffer, info.m_length);
}

MemoryService::MemoryService(LogicalState& t_lState, PresetBank& t_presetBank)
  : m_logicalState(t_lState),
    m_presetBank(t_presetBank),
    m_eeprom(nullptr) {}

MemoryService::MemoryService(LogicalState& t_lState, PresetBank& t_presetBank, MockEEPROM* t_eeprom)
  : m_logicalState(t_lState),
    m_presetBank(t_presetBank),
    m_eeprom(t_eeprom) {}

void MemoryService::init() {

}

void MemoryService::handleEvent(const Event& t_event) {
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

    case EventType::kSaveLogicalState:
      saveRegion(MemoryRegion::kLogicalState);
      break;

    case EventType::kRestoreState:
      loadRegion(MemoryRegion::kLogicalState);
      break;

    case EventType::kLoadPresetBank:
      loadPresetBank(t_event.m_data.value);
      EventBus::publish({EventType::kPresetBankLoaded, t_event.m_timestamp /*millis()*/, {.value=t_event.m_data.value}});
      break;

    case EventType::kSavePreset: {
      uint8_t low = 0, high = 0;
      Utils::pack16(t_event.m_data.value, low, high);
      savePreset(high, low);
      break;
    }

    default:
      break;
  }
}

void MemoryService::update() {

}

bool MemoryService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kSaveEvent
      || t_category == EventCategory::kLoadEvent
      || t_category == EventCategory::kBootEvent
      || t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPresetBankChangedEvent;
}
