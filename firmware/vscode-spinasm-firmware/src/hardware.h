#pragma once

#include <Arduino.h>
#include "programmer.h"
#include "eeprom.h"

namespace MessageLength {
  constexpr uint8_t c_orderMessageLength = 1;
  constexpr uint8_t c_addressMessageLength = 2;
  constexpr uint8_t c_dataMessageLength = 32;
}

enum SystemState {
  kReceivingMessage,
  kProcessingMessage,
  kProcessingWriteMessage,
  kProcessingReadMessage,
};

enum class Message : uint8_t {
  kNone,
  kRuThere,
  kRuReady,
  kRead,
  kWrite,
  kEnd,
  kNok,
  kOk,
  kTimeout,
  kWriteError,
  kReadError,
  kCommunicationError,
  kFramingError
};

struct OperationContext {
  uint16_t address = 0;
  uint8_t buffer[MessageLength::c_dataMessageLength] = {0};
  size_t bufferLength = 0;

  void reset() {
    address = 0;
    bufferLength = 0;
    memset(buffer, 0, sizeof(buffer));
  }
};

class Hardware {
private:
  SystemState m_systemState = SystemState::kReceivingMessage;
  Message m_currentMessage = Message::kNone;
  OperationContext m_context;

  ProgrammerStatus getProgrammerMessage(uint8_t* t_data, uint8_t t_count);
  EEPROMResult readEpromPage(uint16_t t_address, uint8_t* t_buffer, size_t t_length);
  EEPROMResult WriteEpromPage(uint16_t t_address, uint8_t* t_buffer, size_t t_length);

  Message validateMessage(uint8_t t_value);
  void sendOrder(Message t_order);

  void transitionToState(SystemState t_state);
  void processReceivingMessage();
  void processProcessingMessage();

  void processRuThereMessage();
  void processRuReadyMessage();
  void processReadMessage();
  void processWriteMessage();
  void processEndMessage();

public:
  void setup();
  void poll();
  void process();
};
