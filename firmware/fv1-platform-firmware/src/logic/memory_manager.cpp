#include "logic/memory_manager.h"

void MemoryManager::serializeDeviceState(const LogicalState& t_state, uint8_t* t_buffer) const {
  // Device state basic data
  t_buffer[0] = static_cast<uint8_t>(t_state.m_bypassState);
  t_buffer[1] = static_cast<uint8_t>(t_state.m_programMode);
  t_buffer[2] = t_state.m_currentProgram;
  t_buffer[3] = t_state.m_currentPreset;
  t_buffer[4] = t_state.m_midiChannel;

  // Tap handler data
  t_buffer[5] = static_cast<uint8_t>(t_state.m_tapState);
  t_buffer[6] = static_cast<uint8_t>(t_state.m_divState);
  t_buffer[7] = static_cast<uint8_t>(t_state.m_divValue);

  uint8_t low = 0, high = 0;

  Utils::pack16(t_state.m_interval, low, high);
  t_buffer[8] = low;
  t_buffer[9] = high;

  Utils::pack16(t_state.m_divInterval, low, high);
  t_buffer[10] = low;
  t_buffer[11] = high;

  // Expr handlers data
  for (uint8_t i = 0; i < ProgramConstants::c_maxPrograms; i++) {
    uint8_t handlerOffset = MemoryManagerConstants::c_stateSize + MemoryManagerConstants::c_tapSize + (i * MemoryManagerConstants::c_exprSize); // Starts at byte 11 and 7 bytes per handler

    t_buffer[handlerOffset] = static_cast<uint8_t>(t_state.m_exprParams[i].m_state);
    t_buffer[handlerOffset + 1] = static_cast<uint8_t>(t_state.m_exprParams[i].m_mappedPot);
    t_buffer[handlerOffset + 2] = static_cast<uint8_t>(t_state.m_exprParams[i].m_direction);

    Utils::pack16(t_state.m_exprParams[i].m_heelValue, low, high);
    t_buffer[handlerOffset + 3] = low;
    t_buffer[handlerOffset + 4] = high;

    Utils::pack16(t_state.m_exprParams[i].m_toeValue, low, high);
    t_buffer[handlerOffset + 5] = low;
    t_buffer[handlerOffset + 6] = high;
  }
}

void MemoryManager::deserializeDeviceState(LogicalState& t_state, const uint8_t* t_buffer) const {
  // Device state basic data
  t_state.m_bypassState = (static_cast<BypassState>(t_buffer[0]));
  t_state.m_programMode = (static_cast<ProgramMode>(t_buffer[1]));
  t_state.m_currentProgram = (t_buffer[2]);
  t_state.m_currentPreset = (t_buffer[3]);
  t_state.m_midiChannel = (t_buffer[4]);

  // Tap handler data
  t_state.m_tapState = (static_cast<TapState>(t_buffer[5]));
  t_state.m_divState = (static_cast<DivState>(t_buffer[6]));
  t_state.m_divValue = (static_cast<DivValue>(t_buffer[7]));

  Utils::unpack16(t_buffer[8], t_buffer[9], t_state.m_interval);
  Utils::unpack16(t_buffer[10], t_buffer[11], t_state.m_divInterval);

  // Expr handlers data
  for (uint8_t i = 0; i < ProgramConstants::c_maxPrograms; i++) {
    uint8_t handlerOffset = MemoryManagerConstants::c_stateSize + MemoryManagerConstants::c_tapSize + (i * MemoryManagerConstants::c_exprSize); // Starts at byte 11 and 7 bytes per handler

    t_state.m_exprParams[i].m_state = (static_cast<ExprState>(t_buffer[handlerOffset]));
    t_state.m_exprParams[i].m_mappedPot = (static_cast<MappedPot>(t_buffer[handlerOffset + 1]));
    t_state.m_exprParams[i].m_direction = (static_cast<Direction>(t_buffer[handlerOffset + 2]));

    Utils::unpack16(t_buffer[handlerOffset + 3], t_buffer[handlerOffset + 4], t_state.m_exprParams[i].m_heelValue);
    Utils::unpack16(t_buffer[handlerOffset + 5], t_buffer[handlerOffset + 6], t_state.m_exprParams[i].m_toeValue);
  }
}

void MemoryManager::setup() {
  eeprom.setup();
}

bool MemoryManager::isMemoryInitialized() {
  return static_cast<bool>(eeprom.readInt8(MemoryManagerConstants::c_memoryInitializedAddress));
}

void MemoryManager::saveDeviceState(const LogicalState& t_state) {
  uint16_t address = MemoryManagerConstants::c_deviceStateStartAddress;
  uint8_t buffer[MemoryManagerConstants::c_deviceStateSize]{0};

  serializeDeviceState(t_state, buffer);

  for (uint16_t i = 0; i < MemoryManagerConstants::c_deviceStateSize; i++) {
    eeprom.writeInt8(address + i, buffer[i]);
  }
}

void MemoryManager::loadDeviceState(LogicalState& t_state) {
  uint16_t address = MemoryManagerConstants::c_deviceStateStartAddress;
  uint8_t buffer[MemoryManagerConstants::c_deviceStateSize]{0};

  for (uint16_t i = 0; i < MemoryManagerConstants::c_deviceStateSize; i++) {
    buffer[i] = eeprom.readInt8(address + i);
  }

  deserializeDeviceState(t_state, buffer);
}
