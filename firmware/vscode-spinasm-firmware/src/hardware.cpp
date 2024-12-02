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

void Hardware::transitionToState(SystemState t_state) {
  switch (t_state) {
    case SystemState::kReceivingMessage:
      m_systemState = kReceivingMessage;
      break;

    case SystemState::kProcessingMessage:
      m_systemState = kProcessingMessage;
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
      processReadMessage();
      break;

    case Message::kWrite:
      transitionToState(SystemState::kProcessingWriteMessage);
      break;

    case Message::kEnd:
      transitionToState(SystemState::kProcessingReadMessage);
      break;

    default:
      transitionToState(SystemState::kReceivingMessage);
      break;
  }
}

void Hardware::processRuThereMessage() {
  uint8_t message[MessageLength::c_orderMessageLength] = {static_cast<uint8_t>(Message::kOk)};
  programmer.sendMessage(message, MessageLength::c_orderMessageLength);

  transitionToState(kReceivingMessage);
}

void Hardware::processRuReadyMessage() {
  uint8_t message[MessageLength::c_orderMessageLength] = {0};

  if (eeprom.isReady()) {
    message[0] = static_cast<uint8_t>(Message::kOk);
    programmer.sendMessage(message, MessageLength::c_orderMessageLength);
  }
  else {
    message[0] = static_cast<uint8_t>(Message::kNok);
    programmer.sendMessage(message, MessageLength::c_orderMessageLength);
  }

  transitionToState(kReceivingMessage);
}

void Hardware::processReadMessage() {
  
}

void Hardware::processWriteMessage() {
  
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