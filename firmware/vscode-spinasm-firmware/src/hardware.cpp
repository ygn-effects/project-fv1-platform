#include "hardware.h"

EEPROM eeprom(0x50);
Programmer programmer(9);

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
      break;

    default:
      return Message::kNone;
      break;
  }
}

void Hardware::sendOrder(Message t_order) {
  uint8_t order[MessageLength::c_orderMessageLength] = {static_cast<uint8_t>(t_order)};
  programmer.sendMessage(order, MessageLength::c_orderMessageLength);
}

void Hardware::sendOk() {
  sendOrder(Message::kOk);
}

void Hardware::sendNok() {
  sendOrder(Message::kNok);
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
      break;
  }
}

void Hardware::processReceivingMessage() {
  uint8_t message[MessageLength::c_orderMessageLength] = {0};

  if (programmer.getMessage(message, MessageLength::c_orderMessageLength)) {
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
  sendOk();

  transitionToState(kReceivingMessage);
}

void Hardware::processRuReadyMessage() {
  if (eeprom.isReady()) {
    sendOk();
  }
  else {
    sendNok();
  }

  transitionToState(kReceivingMessage);
}

void Hardware::processReadMessage() {
  if (!m_context.address) {
    uint8_t address[MessageLength::c_addressMessageLength];

    if (programmer.getMessage(address, MessageLength::c_addressMessageLength)) {
      m_context.address = (address[0] << 8) | address[1];

      sendOk();
    }
    else {
      sendNok();
      m_context.reset();

      transitionToState(kReceivingMessage);
    }
  }
  else {
    m_context.bufferLength = MessageLength::c_dataMessageLength;
    eeprom.readPage(m_context.address, m_context.buffer, m_context.bufferLength);
    programmer.sendMessage(m_context.buffer, m_context.bufferLength);
    sendOk();
    m_context.reset();

    transitionToState(kReceivingMessage);
  }
}

void Hardware::processWriteMessage() {
  if (!m_context.address) {
    uint8_t address[MessageLength::c_addressMessageLength];

    if (programmer.getMessage(address, MessageLength::c_addressMessageLength)) {
      m_context.address = (address[0] << 8) | address[1];

      sendOk();
    }
    else {
      sendNok();
      m_context.reset();

      transitionToState(kReceivingMessage);
    }
  }
  else if (m_context.bufferLength == 0) {
    m_context.bufferLength = MessageLength::c_dataMessageLength;

    if (programmer.getMessage(m_context.buffer, m_context.bufferLength)) {
      sendOk();
    }
    else {
      sendNok();
      m_context.reset();

      transitionToState(kReceivingMessage);
    }
  }
  else {
    if (eeprom.writePage(m_context.address, m_context.buffer, m_context.bufferLength) == 0) {
      sendOk();
      m_context.reset();

      transitionToState(kReceivingMessage);
    }
    else {
      sendNok();
      m_context.reset();

      transitionToState(kReceivingMessage);
    }
  }
}

void Hardware::processEndMessage() {
  uint8_t message[MessageLength::c_orderMessageLength] = {static_cast<uint8_t>(Message::kOk)};
  programmer.sendMessage(message, MessageLength::c_orderMessageLength);

  transitionToState(kReceivingMessage);
}

void Hardware::setup() {
  eeprom.setup();
  programmer.setup();
}

void Hardware::poll() {
  switch (m_systemState) {
    case SystemState::kReceivingMessage:
    case SystemState::kProcessingMessage:
    case SystemState::kProcessingReadMessage:
    case SystemState::kProcessingWriteMessage:
      programmer.receiveData();
      break;

    default:
      break;
  }
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