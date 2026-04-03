#pragma once

#include <stdint.h>
#include "utils/circular_buffer.h"

namespace MidiHandlerConstants {
  constexpr uint8_t c_maxMidiChannels = 15;
  constexpr uint8_t c_maxMidiMessages = 5;
}

namespace MidiMessageMasks {
  constexpr uint8_t c_statusMask = 0x80;
  constexpr uint8_t c_messageTypeMask = 0x70;
  constexpr uint8_t c_channelMask = 0xF;
}

namespace MidiMessageConstants {
  constexpr uint8_t c_minValue = 0;
  constexpr uint8_t c_maxValue = 127;
}

enum class MidiMessageType : uint8_t {
  kControlChange = 0x30,
  kProgramChange = 0x40
};

struct MidiMessage {
  MidiMessageType m_type;
  uint8_t m_channel;
  uint8_t m_param;
  uint8_t m_value;
};

class MidiHandler {
  private:
    CircularBuffer<MidiMessage, MidiHandlerConstants::c_maxMidiMessages> m_messageBuffer;

    uint8_t m_dataBuffer[2];
    uint8_t m_currentChannel = 0;
    uint8_t m_runningStatus = 0;
    uint8_t m_expectedDataCount = 0;
    uint8_t m_dataCount = 0;

  public:
    void pushByte(uint8_t t_byte);

    bool popMessage(MidiMessage& t_message);

    void setMidiChannel(uint8_t t_channel);
};

