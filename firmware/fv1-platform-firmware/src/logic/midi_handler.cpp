#include "logic/midi_handler.h"

void MidiHandler::pushByte(uint8_t t_byte) {
  if (t_byte >= MidiMessageMasks::c_statusMask && (t_byte & MidiMessageMasks::c_channelMask) != m_currentChannel) { return; }
  if (t_byte < MidiMessageMasks::c_statusMask && m_runningStatus == 0) { return; }

  if (t_byte >= MidiMessageMasks::c_statusMask) {
    switch (t_byte & MidiMessageMasks::c_messageTypeMask) {
      case static_cast<uint8_t>(MidiMessageType::kControlChange):
        m_runningStatus = static_cast<uint8_t>(MidiMessageType::kControlChange);
        m_expectedDataCount = 2;
        m_dataCount = 0;
        break;

      case static_cast<uint8_t>(MidiMessageType::kProgramChange):
        m_runningStatus = static_cast<uint8_t>(MidiMessageType::kProgramChange);
        m_expectedDataCount = 1;
        m_dataCount = 0;
        break;

      default:
        m_runningStatus = 0;
        break;
    }
  }
  else {
    m_dataBuffer[m_dataCount++] = t_byte;

    if (m_dataCount == m_expectedDataCount) {
      MidiMessage message = {
        .m_type = static_cast<MidiMessageType>(m_runningStatus),
        .m_channel = m_currentChannel,
        .m_param = m_dataBuffer[1],
        .m_value = m_expectedDataCount > 1 ? m_dataBuffer[1] : 0
      };

      m_messageBuffer.push(message);
      m_dataCount = 0;
      m_runningStatus = 0;
    }
  }
}

bool MidiHandler::popMessage(MidiMessage& t_message) {
  return m_messageBuffer.pop(t_message);
}

void MidiHandler::setMidiChannel(uint8_t t_channel) {
  m_currentChannel = t_channel;
}
