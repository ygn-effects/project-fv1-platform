#include "logic/memory_handler.h"

void MemoryHandler::serializeBypass(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex) {
  t_buffer[t_startIndex] = static_cast<uint8_t>(t_lState.m_bypassState);
}

void MemoryHandler::serializeProgramMode(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex) {
  t_buffer[t_startIndex] = static_cast<uint8_t>(t_lState.m_programMode);
}

void MemoryHandler::serializeCurrentProgram(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex) {
  t_buffer[t_startIndex] = static_cast<uint8_t>(t_lState.m_currentProgram);
}

void MemoryHandler::serializeCurrentPreset(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex) {
  t_buffer[t_startIndex] = static_cast<uint8_t>(t_lState.m_currentPreset);
}

void MemoryHandler::serializeMidiChannel(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex) {
  t_buffer[t_startIndex] = static_cast<uint8_t>(t_lState.m_midiChannel);
}

void MemoryHandler::serializeDeviceState(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex) {
  serializeBypass(t_lState, t_buffer, t_startIndex);
  serializeProgramMode(t_lState, t_buffer, t_startIndex + 1);
  serializeCurrentProgram(t_lState, t_buffer, t_startIndex + 2);
  serializeCurrentPreset(t_lState, t_buffer, t_startIndex + 3);
  serializeMidiChannel(t_lState, t_buffer, t_startIndex + 4);
}

void MemoryHandler::serializeTap(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex) {
  t_buffer[t_startIndex] = static_cast<uint8_t>(t_lState.m_tapState);
  t_buffer[t_startIndex + 1] = static_cast<uint8_t>(t_lState.m_divState);
  t_buffer[t_startIndex + 2] = static_cast<uint8_t>(t_lState.m_divValue);

  uint8_t low = 0, high = 0;
  Utils::pack16(t_lState.m_interval, low, high);
  t_buffer[t_startIndex + 3] = low;
  t_buffer[t_startIndex + 4] = high;

  Utils::pack16(t_lState.m_divInterval, low, high);
  t_buffer[t_startIndex + 5] = low;
  t_buffer[t_startIndex + 6] = high;
}

void MemoryHandler::serializeTempo(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex) {
  uint8_t low = 0, high = 0;
  Utils::pack16(t_lState.m_tempo, low, high);
  t_buffer[t_startIndex] = low;
  t_buffer[t_startIndex + 1] = high;
}

void MemoryHandler::serializeExprParam(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_programIndex) {
  t_buffer[t_startIndex] = static_cast<uint8_t>(t_lState.m_exprParams[t_programIndex].m_state);
  t_buffer[t_startIndex + 1] = static_cast<uint8_t>(t_lState.m_exprParams[t_programIndex].m_mappedPot);
  t_buffer[t_startIndex + 2] = static_cast<uint8_t>(t_lState.m_exprParams[t_programIndex].m_direction);

  uint8_t low = 0, high = 0;
  Utils::pack16(t_lState.m_exprParams[t_programIndex].m_heelValue, low, high);
  t_buffer[t_startIndex + 3] = low;
  t_buffer[t_startIndex + 4] = high;

  Utils::pack16(t_lState.m_exprParams[t_programIndex].m_toeValue, low, high);
  t_buffer[t_startIndex + 5] = low;
  t_buffer[t_startIndex + 6] = high;
}

void MemoryHandler::serializePotParam(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_potIndex) {
  t_buffer[t_startIndex] = static_cast<uint8_t>(t_lState.m_potParams[t_potIndex].m_state);

  uint8_t low = 0, high = 0;
  Utils::pack16(t_lState.m_potParams[t_potIndex].m_value, low, high);
  t_buffer[t_startIndex + 1] = low;
  t_buffer[t_startIndex + 2] = high;

  Utils::pack16(t_lState.m_potParams[t_potIndex].m_minValue, low, high);
  t_buffer[t_startIndex + 3] = low;
  t_buffer[t_startIndex + 4] = high;

  Utils::pack16(t_lState.m_potParams[t_potIndex].m_maxValue, low, high);
  t_buffer[t_startIndex + 5] = low;
  t_buffer[t_startIndex + 6] = high;
}

void MemoryHandler::deserializeBypass(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex) {
  t_lState.m_bypassState = LogicalStateValidator::setSafeBypassState(t_buffer[t_startIndex]);
}

void MemoryHandler::deserializeProgramMode(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex) {
  t_lState.m_programMode = LogicalStateValidator::setSafeProgramMode(t_buffer[t_startIndex]);
}

void MemoryHandler::deserializeCurrentProgram(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex) {
  t_lState.m_currentProgram = Utils::clamp<uint8_t>(t_buffer[t_startIndex], 0, ProgramConstants::c_maxPrograms);
}

void MemoryHandler::deserializeCurrentPreset(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex) {
  t_lState.m_currentPreset = Utils::clamp<uint8_t>(t_buffer[t_startIndex], 0, PresetConstants::c_maxPreset);
}

void MemoryHandler::deserializeMidiChannel(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex) {
  t_lState.m_midiChannel = Utils::clamp<uint8_t>(t_buffer[t_startIndex], 0, 7);
}

void MemoryHandler::deserializeDeviceState(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex) {
  deserializeBypass(t_lState, t_buffer, t_startIndex);
  deserializeProgramMode(t_lState, t_buffer, t_startIndex + 1);
  deserializeCurrentProgram(t_lState, t_buffer, t_startIndex + 2);
  deserializeCurrentPreset(t_lState, t_buffer, t_startIndex + 3);
  deserializeMidiChannel(t_lState, t_buffer, t_startIndex + 4);
}

void MemoryHandler::deserializeTap(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex) {
  t_lState.m_tapState = LogicalStateValidator::setSafeTapState(t_buffer[t_startIndex]);
  t_lState.m_divState = LogicalStateValidator::setSafeDivState(t_buffer[t_startIndex + 1]);
  t_lState.m_divValue = LogicalStateValidator::setSafeDivValue(t_buffer[t_startIndex + 2]);

  uint16_t interval = 0;
  Utils::unpack16(t_buffer[t_startIndex + 3], t_buffer[t_startIndex + 4], interval);
  // Magic number
  t_lState.m_interval = Utils::clamp<uint16_t>(interval, 0, 1000);

  uint16_t divInterval = 0;
  Utils::unpack16(t_buffer[t_startIndex + 5], t_buffer[t_startIndex + 6], divInterval);
  // Magic number
  t_lState.m_divInterval = Utils::clamp<uint16_t>(divInterval, 0, 1000);
}

void MemoryHandler::deserializeTempo(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex) {
  uint16_t tempo = 0;
  Utils::unpack16(t_buffer[t_startIndex], t_buffer[t_startIndex + 1], tempo);
  // Magic number
  t_lState.m_tempo = Utils::clamp<uint16_t>(tempo, 0, 1000);
}

void MemoryHandler::deserializeExprParam(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_programIndex) {
  t_lState.m_exprParams[t_programIndex].m_state = LogicalStateValidator::setSafeExprState(t_buffer[t_startIndex]);
  t_lState.m_exprParams[t_programIndex].m_mappedPot = LogicalStateValidator::setSafeMappedPot(t_buffer[t_startIndex + 1]);
  t_lState.m_exprParams[t_programIndex].m_direction = LogicalStateValidator::setSafeDirection(t_buffer[t_startIndex + 2]);

  uint16_t heelValue = 0;
  Utils::unpack16(t_buffer[t_startIndex + 3], t_buffer[t_startIndex + 4], heelValue);
  // Magic number
  t_lState.m_exprParams[t_programIndex].m_heelValue = Utils::clamp<uint16_t>(heelValue, 0, 1023);

  uint16_t toeValue = 0;
  Utils::unpack16(t_buffer[t_startIndex + 5], t_buffer[t_startIndex + 6], toeValue);
  // Magic number
  t_lState.m_exprParams[t_programIndex].m_toeValue = Utils::clamp<uint16_t>(toeValue, 0, 1023);
}

void MemoryHandler::deserializePotParam(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_potIndex) {
  t_lState.m_potParams[t_potIndex].m_state = LogicalStateValidator::setSafePotState(t_buffer[t_startIndex]);

  uint16_t value = 0;
  Utils::unpack16(t_buffer[t_startIndex + 1], t_buffer[t_startIndex + 2], value);
  t_lState.m_potParams[t_potIndex].m_value = Utils::clamp<uint16_t>(value, 0, 1023);

  uint16_t minValue = 0;
  Utils::unpack16(t_buffer[t_startIndex + 3], t_buffer[t_startIndex + 4], minValue);
  t_lState.m_potParams[t_potIndex].m_minValue = Utils::clamp<uint16_t>(minValue, 0, 1023);

  uint16_t maxValue = 0;
  Utils::unpack16(t_buffer[t_startIndex + 5], t_buffer[t_startIndex + 6], maxValue);
  t_lState.m_potParams[t_potIndex].m_maxValue = Utils::clamp<uint16_t>(maxValue, 0, 1023);
}

RegionInfo MemoryHandler::calculateRegionInfo(MemoryRegion t_region, uint8_t t_programIndex, uint8_t t_index) {
  RegionInfo info = {0, 0};

  switch (t_region) {
    case MemoryRegion::kBypass:
      info = {
        .m_address = MemoryLayout::c_bypassState,
        .m_length = 1
      };
      break;

    case MemoryRegion::kProgramMode:
      info = {
        .m_address = MemoryLayout::c_programMode,
        .m_length = 1
      };
      break;

    case MemoryRegion::kCurrentProgram:
      info = {
        .m_address = MemoryLayout::c_currentProgram,
        .m_length = 1
      };
      break;

    case MemoryRegion::kCurrentPreset:
      info = {
        .m_address = MemoryLayout::c_currentPreset,
        .m_length = 1
      };
      break;

    case MemoryRegion::kMidiChannel:
      info = {
        .m_address = MemoryLayout::c_midiChannel,
        .m_length = 1
      };
      break;

    case MemoryRegion::kDeviceState:
      info = {
        .m_address = MemoryLayout::c_deviceStateStart,
        .m_length = MemoryLayout::c_deviceStateSize
      };
      break;

    case MemoryRegion::kTap:
      info = {
        .m_address = MemoryLayout::c_tapStart,
        .m_length = MemoryLayout::c_tapSize
      };
      break;

    case MemoryRegion::kTempo:
      info = {
        .m_address = MemoryLayout::c_tempoStart,
        .m_length = MemoryLayout::c_tempoSize
      };
      break;

    case MemoryRegion::kExpr:
      info = {
        .m_address = MemoryLayout::getExprParamOffset(t_programIndex),
        .m_length = MemoryLayout::c_exprSize
      };
      break;

    case MemoryRegion::kPot:
      info = {
        .m_address = MemoryLayout::getPotParamOffset(t_programIndex, t_index),
        .m_length = MemoryLayout::c_potParamSize
      };
      break;

    default:
      return info;
  }

  return info;
}

void MemoryHandler::serializeRegion(MemoryRegion t_region, const LogicalState& t_lState, uint8_t* t_buffer, uint8_t t_programIndex, uint8_t t_index) {
  switch (t_region) {
    case MemoryRegion::kBypass:
      serializeBypass(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kProgramMode:
      serializeProgramMode(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kCurrentProgram:
      serializeCurrentProgram(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kCurrentPreset:
      serializeCurrentPreset(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kMidiChannel:
      serializeMidiChannel(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kDeviceState:
      serializeDeviceState(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kTap:
      serializeTap(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kTempo:
      serializeTempo(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kExpr:
      serializeExprParam(t_lState, t_buffer, 0, t_programIndex);
      break;

    case MemoryRegion::kPot:
      serializePotParam(t_lState, t_buffer, 0, t_index);
      break;

    default:
      break;
  }
}

void MemoryHandler::deserializeRegion(MemoryRegion t_region, LogicalState& t_lState, const uint8_t* t_buffer, uint8_t t_programIndex, uint8_t t_index) {
  switch (t_region) {
    case MemoryRegion::kBypass:
      deserializeBypass(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kProgramMode:
      deserializeProgramMode(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kCurrentProgram:
      deserializeCurrentProgram(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kCurrentPreset:
      deserializeCurrentPreset(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kMidiChannel:
      deserializeMidiChannel(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kDeviceState:
      deserializeDeviceState(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kTap:
      deserializeTap(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kTempo:
      deserializeTempo(t_lState, t_buffer, 0);
      break;

    case MemoryRegion::kExpr:
      deserializeExprParam(t_lState, t_buffer, 0, t_programIndex);
      break;

    case MemoryRegion::kPot:
      deserializePotParam(t_lState, t_buffer, 0, t_index);
      break;

    default:
      break;
  }
}

