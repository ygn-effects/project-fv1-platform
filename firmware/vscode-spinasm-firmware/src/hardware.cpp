#include "hardware.h"

EEPROM eeprom(0x50);
Programmer programmer(9);

ProgrammerStatus Hardware::getProgrammerMessage(uint8_t* t_data, uint8_t t_count) {
  ProgrammerStatus status = programmer.getMessage(t_data, t_count);

  // Handle communication errors explicitly
  if (status != ProgrammerStatus::Success && status != ProgrammerStatus::NoMessage) {
    switch (status) {
      case ProgrammerStatus::FramingError:
        sendOrder(Message::kFramingError);
        break;

      case ProgrammerStatus::Timeout:
        sendOrder(Message::kTimeout);
        break;

      default:
        sendOrder(Message::kCommunicationError);
        break;
    }

    // Reset state after error
    m_context.reset();
    transitionToState(kReceivingMessage);
  }

  return status;
}

EEPROMResult Hardware::readEepromPage(uint16_t t_address, uint8_t* t_buffer, size_t t_length) {
  EEPROMResult result = eeprom.readPage(t_address, t_buffer, t_length);

  // Handle EEPROM read errors explicitly
  if (result != EEPROMResult::Success) {
    switch (result) {
      case EEPROMResult::Timeout:
        sendOrder(Message::kTimeout);
        break;

      case EEPROMResult::ReadError:
        sendOrder(Message::kReadError);
        break;

      default:
        sendOrder(Message::kCommunicationError);
        break;
    }

    // Reset state after error
    m_context.reset();
    transitionToState(kReceivingMessage);
  }

  return result;
}

EEPROMResult Hardware::writeEepromPage(uint16_t t_address, uint8_t* t_buffer, size_t t_length) {
  EEPROMResult result = eeprom.writePage(t_address, t_buffer, t_length);

  // Handle EEPROM write errors explicitly
  if (result != EEPROMResult::Success) {
    switch (result) {
      case EEPROMResult::Timeout:
        sendOrder(Message::kTimeout);
        break;

      case EEPROMResult::WriteError:
        sendOrder(Message::kWriteError);
        break;

      default:
        sendOrder(Message::kCommunicationError);
        break;
    }

    // Reset state after error
    m_context.reset();
    transitionToState(kReceivingMessage);
  }

  return result;
}

Message Hardware::validateMessage(uint8_t t_value) {
  switch (static_cast<Message>(t_value)) {
    case Message::kRuThere:
    case Message::kRuReady:
    case Message::kRead:
    case Message::kWrite:
    case Message::kEnd:
    case Message::kNok:
    case Message::kOk:
      return static_cast<Message>(t_value);

    default:
      return Message::kNone;
  }
}

void Hardware::sendOrder(Message t_order) {
  uint8_t order[MessageLength::c_orderMessageLength] = {static_cast<uint8_t>(t_order)};
  programmer.sendMessage(order, MessageLength::c_orderMessageLength);
}

void Hardware::transitionToState(SystemState t_state) {
  switch (t_state) {
    case SystemState::kReceivingMessage:
      m_systemState = kReceivingMessage;
      break;

    case SystemState::kProcessingMessage:
      m_systemState = kProcessingMessage;
      break;

    case SystemState::kProcessingReadMessage:
      m_systemState = kProcessingReadMessage;
      break;

    case SystemState::kProcessingWriteMessage:
      m_systemState = kProcessingWriteMessage;
      break;

    default:
      m_systemState = kReceivingMessage;
      break;
  }
}

void Hardware::processReceivingMessage() {
  uint8_t message[MessageLength::c_orderMessageLength] = {0};

  // Receive and validate incoming message
  ProgrammerStatus status = getProgrammerMessage(message, MessageLength::c_orderMessageLength);
  if (status == ProgrammerStatus::Success) {
    m_currentMessage = validateMessage(message[0]);
    transitionToState(SystemState::kProcessingMessage);
  }
}

void Hardware::processProcessingMessage() {
  switch (m_currentMessage) {
    case Message::kRuThere:
      processRuThereMessage();
      break;

    case Message::kRuReady:
      processRuReadyMessage();
      break;

    case Message::kRead:
      transitionToState(SystemState::kProcessingReadMessage);
      break;

    case Message::kWrite:
      transitionToState(SystemState::kProcessingWriteMessage);
      break;

    case Message::kEnd:
      processEndMessage();
      break;

    default:
      transitionToState(SystemState::kReceivingMessage);
      break;
  }
}

void Hardware::processRuThereMessage() {
  sendOrder(Message::kOk);
  transitionToState(kReceivingMessage);
}

void Hardware::processRuReadyMessage() {
  sendOrder(eeprom.isReady() ? Message::kOk : Message::kNok);
  transitionToState(kReceivingMessage);
}

void Hardware::processReadMessage() {
  if (!m_context.address) {
    // Get EEPROM address to read from
    uint8_t address[MessageLength::c_addressMessageLength];
    ProgrammerStatus status = getProgrammerMessage(address, MessageLength::c_addressMessageLength);

    if (status == ProgrammerStatus::Success) {
      m_context.address = (address[0] << 8) | address[1];
      sendOrder(Message::kOk);
    }
  }
  else {
    // Perform EEPROM read and send data back
    m_context.bufferLength = MessageLength::c_dataMessageLength;
    EEPROMResult result = readEepromPage(m_context.address, m_context.buffer, m_context.bufferLength);

    if (result == EEPROMResult::Success) {
      programmer.sendMessage(m_context.buffer, m_context.bufferLength);
      sendOrder(Message::kOk);
      m_context.reset();
      transitionToState(kReceivingMessage);
    }
  }
}

void Hardware::processWriteMessage() {
  if (!m_context.address) {
    // Get EEPROM address to write to
    uint8_t address[MessageLength::c_addressMessageLength];
    ProgrammerStatus status = getProgrammerMessage(address, MessageLength::c_addressMessageLength);

    if (status == ProgrammerStatus::Success) {
      m_context.address = (address[0] << 8) | address[1];
      sendOrder(Message::kOk);
    }
  }
  else if (m_context.bufferLength == 0) {
    // Get data to write to EEPROM
    m_context.bufferLength = MessageLength::c_dataMessageLength;
    ProgrammerStatus status = getProgrammerMessage(m_context.buffer, m_context.bufferLength);

    if (status == ProgrammerStatus::Success) {
      sendOrder(Message::kOk);
    }
  }
  else {
    // Perform EEPROM write operation
    EEPROMResult result = writeEepromPage(m_context.address, m_context.buffer, m_context.bufferLength);

    if (result == EEPROMResult::Success) {
      sendOrder(Message::kOk);
      m_context.reset();
      transitionToState(kReceivingMessage);
    }
  }
}

void Hardware::processEndMessage() {
  sendOrder(Message::kOk);
  transitionToState(kReceivingMessage);
}

void Hardware::setup() {
  eeprom.setup();
  programmer.setup();
}

void Hardware::process() {
  switch (m_systemState) {
    case SystemState::kReceivingMessage:
      processReceivingMessage();
      break;

    case SystemState::kProcessingMessage:
      processProcessingMessage();
      break;

    case SystemState::kProcessingWriteMessage:
      processWriteMessage();
      break;

    case SystemState::kProcessingReadMessage:
      processReadMessage();
      break;

    default:
      break;
  }
}
