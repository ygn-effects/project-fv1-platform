#pragma once

#include <Arduino.h>
#include "programmer.h"
#include "eeprom.h"

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
  kOk
};

struct OperationContext {
  uint16_t address = 0;
  uint8_t buffer[64] = {0};
  size_t bufferLength = 0;
};

class Hardware {
  private:
    SystemState m_systemState = SystemState::kReceivingMessage;
    Message m_currentMessage = Message::kNone;
    OperationContext m_context;

    Message validateMessage(uint8_t t_value);
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