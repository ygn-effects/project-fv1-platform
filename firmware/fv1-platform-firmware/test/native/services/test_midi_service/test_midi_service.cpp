#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/midi_service.h"

#include "../src/services/midi_service.cpp"
#include "../src/logic/midi_handler.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_basic_cc_message() {
  LogicalState logicalState;
  MidiService midiService(logicalState);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x00);
  midiHandler->pushByte(0x40);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMidiPot0Moved, e.m_type);
  TEST_ASSERT_EQUAL(64, e.m_data.value);
}

void test_successive_cc_messages() {
  LogicalState logicalState;
  MidiService midiService(logicalState);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x00);
  midiHandler->pushByte(0x40);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x01);
  midiHandler->pushByte(0x7F);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMidiPot0Moved, e.m_type);
  TEST_ASSERT_EQUAL(64, e.m_data.value);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMidiPot1Moved, e.m_type);
  TEST_ASSERT_EQUAL(127, e.m_data.value);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x02);
  midiHandler->pushByte(0x20);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMidiPot2Moved, e.m_type);
  TEST_ASSERT_EQUAL(32, e.m_data.value);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x04);
  midiHandler->pushByte(0x00);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMidiBypassPressed, e.m_type);
  TEST_ASSERT_EQUAL(0, e.m_data.value);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x04);
  midiHandler->pushByte(0x7F);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMidiBypassPressed, e.m_type);
  TEST_ASSERT_EQUAL(127, e.m_data.value);
}

void test_cc_invalid_data() {
  LogicalState logicalState;
  MidiService midiService(logicalState);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x00);

  midiService.update();

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x02);
  midiHandler->pushByte(0x20);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMidiPot2Moved, e.m_type);
  TEST_ASSERT_EQUAL(32, e.m_data.value);

  midiHandler->pushByte(0xB0);

  midiService.update();

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x03);
  midiHandler->pushByte(0x20);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMidiMixPotMoved, e.m_type);
  TEST_ASSERT_EQUAL(32, e.m_data.value);
}

void test_basic_pc_message() {
  LogicalState logicalState;
  MidiService midiService(logicalState);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  midiHandler->pushByte(0xC0);
  midiHandler->pushByte(0x03);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMidiProgramChanged, e.m_type);
  TEST_ASSERT_EQUAL(3, e.m_data.value);
}

void test_pc_invalid_data() {
  LogicalState logicalState;
  MidiService midiService(logicalState);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  midiHandler->pushByte(0xC0);
  midiHandler->pushByte(0x81);

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_basic_cc_message);
  RUN_TEST(test_successive_cc_messages);
  RUN_TEST(test_cc_invalid_data);
  RUN_TEST(test_basic_pc_message);
  RUN_TEST(test_pc_invalid_data);
  UNITY_END();
}
