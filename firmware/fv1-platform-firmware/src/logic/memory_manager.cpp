#include "logic/memory_manager.h"

void MemoryManager::serializeDeviceState(const DeviceState& t_state, uint8_t* t_buffer) const {
  // Device state basic data
  t_buffer[0] = static_cast<uint8_t>(t_state.getBypassState());
  t_buffer[1] = static_cast<uint8_t>(t_state.getProgramMode());
  t_buffer[2] = t_state.getCurrentProgram();
  t_buffer[3] = t_state.getCurrentPreset();
  t_buffer[4] = t_state.getMidiChannel();

  // Tap handler data
  t_buffer[5] = static_cast<uint8_t>(t_state.getTapHandler().getTapState());
  t_buffer[6] = static_cast<uint8_t>(t_state.getTapHandler().getDivState());
  t_buffer[7] = static_cast<uint8_t>(t_state.getTapHandler().getDivValue());

  uint16_t interval = t_state.getTapHandler().getInterval();
  uint16_t divInterval = t_state.getTapHandler().getDivInterval();
  uint8_t low = 0, high = 0;

  Utils::pack16(interval, low, high);
  t_buffer[8] = low;
  t_buffer[9] = high;

  Utils::pack16(divInterval, low, high);
  t_buffer[10] = low;
  t_buffer[11] = high;

  // Expr handlers data
  for (uint8_t i = 0; i < ProgramConstants::c_maxPrograms; i++) {
    uint8_t handlerOffset = MemoryManagerConstants::c_stateSize + MemoryManagerConstants::c_tapSize + (i * MemoryManagerConstants::c_exprSize); // Starts at byte 11 and 7 bytes per handler

    t_buffer[handlerOffset] = static_cast<uint8_t>(t_state.getExprHandler(i).getState());
    t_buffer[handlerOffset + 1] = static_cast<uint8_t>(t_state.getExprHandler(i).getMappedPot());
    t_buffer[handlerOffset + 2] = static_cast<uint8_t>(t_state.getExprHandler(i).getDirection());

    uint16_t heelValue = t_state.getExprHandler(i).getHeelValue();
    uint16_t toeValue = t_state.getExprHandler(i).getToeValue();

    Utils::pack16(heelValue, low, high);
    t_buffer[handlerOffset + 3] = low;
    t_buffer[handlerOffset + 4] = high;

    Utils::pack16(toeValue, low, high);
    t_buffer[handlerOffset + 5] = low;
    t_buffer[handlerOffset + 6] = high;
  }
}

void MemoryManager::deserializeDeviceState(DeviceState& t_state, const uint8_t* t_buffer) const {
  // Device state basic data
  t_state.setBypassState(static_cast<BypassState>(t_buffer[0]));
  t_state.setProgramMode(static_cast<ProgramMode>(t_buffer[1]));
  t_state.setCurrentProgram(t_buffer[2]);
  t_state.setCurrentPreset(t_buffer[3]);
  t_state.setMidiChannel(t_buffer[4]);

  // Tap handler data
  t_state.getTapHandler().setTapState(static_cast<TapState>(t_buffer[5]));
  t_state.getTapHandler().setDivState(static_cast<DivState>(t_buffer[6]));
  t_state.getTapHandler().setDivValue(static_cast<DivValue>(t_buffer[7]));

  uint16_t interval = 0, divInterval = 0;
  Utils::unpack16(t_buffer[8], t_buffer[9], interval);
  Utils::unpack16(t_buffer[10], t_buffer[11], divInterval);

  t_state.getTapHandler().setInterval(interval);
  t_state.getTapHandler().setDivInterval(divInterval);

  // Expr handlers data
  for (uint8_t i = 0; i < ProgramConstants::c_maxPrograms; i++) {
    uint8_t handlerOffset = MemoryManagerConstants::c_stateSize + MemoryManagerConstants::c_tapSize + (i * MemoryManagerConstants::c_exprSize); // Starts at byte 11 and 7 bytes per handler

    t_state.getExprHandler(i).setState(static_cast<ExprState>(t_buffer[handlerOffset]));
    t_state.getExprHandler(i).setMappedPot(static_cast<MappedPot>(t_buffer[handlerOffset + 1]));
    t_state.getExprHandler(i).setDirection(static_cast<Direction>(t_buffer[handlerOffset + 2]));

    uint16_t heelValue = 0, toeValue = 0;
    Utils::unpack16(t_buffer[handlerOffset + 3], t_buffer[handlerOffset + 4], heelValue);
    Utils::unpack16(t_buffer[handlerOffset + 5], t_buffer[handlerOffset + 6], toeValue);

    t_state.getExprHandler(i).setHeelToeValues(heelValue, toeValue);
  }
}

void MemoryManager::setup() {
  eeprom.setup();
}

bool MemoryManager::isMemoryInitialized() {
  return static_cast<bool>(eeprom.readInt8(MemoryManagerConstants::c_memoryInitializedAddress));
}

void MemoryManager::saveDeviceState(const DeviceState& t_state) {
  uint16_t address = MemoryManagerConstants::c_deviceStateStartAddress;
  uint8_t buffer[MemoryManagerConstants::c_deviceStateSize]{0};

  serializeDeviceState(t_state, buffer);

  for (uint16_t i = 0; i < MemoryManagerConstants::c_deviceStateSize; i++) {
    eeprom.writeInt8(address + i, buffer[i]);
  }
}

void MemoryManager::loadDeviceState(DeviceState& t_state) {
  uint16_t address = MemoryManagerConstants::c_deviceStateStartAddress;
  uint8_t buffer[MemoryManagerConstants::c_deviceStateSize]{0};

  for (uint16_t i = 0; i < MemoryManagerConstants::c_deviceStateSize; i++) {
    buffer[i] = eeprom.readInt8(address + i);
  }

  deserializeDeviceState(t_state, buffer);
}
