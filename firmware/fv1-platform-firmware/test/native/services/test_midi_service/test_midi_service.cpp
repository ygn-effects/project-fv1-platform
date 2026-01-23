#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "mock/mock_clock.h"
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
  MockedClock clock;
  MidiService midiService(logicalState, clock);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  // CC message POT0
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x00);
  midiHandler->pushByte(0x40);

  midiService.update();

  // Should be 1 event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  // Should be POT0 at 64
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(PotId::kPot0));
  TEST_ASSERT_EQUAL(64, e.m_data.value);

  // Check there is no other event
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_successive_cc_messages() {
  LogicalState logicalState;
  MockedClock clock;
  MidiService midiService(logicalState, clock);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  // CC message POT0 64
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x00);
  midiHandler->pushByte(0x40);

  // CC message POT1 127
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x01);
  midiHandler->pushByte(0x7F);

  midiService.update();

  // First event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  // Should be POT0 at 64
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(PotId::kPot0));
  TEST_ASSERT_EQUAL(64, e.m_data.value);

  midiService.update();

  // Second event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  // Should be POT0 at 64
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(PotId::kPot1));
  TEST_ASSERT_EQUAL(127, e.m_data.value);

  // Check there is no other event
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // CC message POT2 at 32
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x02);
  midiHandler->pushByte(0x20);

  midiService.update();

  // First event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  // Should be POT2 at 32
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(PotId::kPot2));
  TEST_ASSERT_EQUAL(32, e.m_data.value);

  // Check there is no other event
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // CC message bypass off
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x04);
  midiHandler->pushByte(0x00);

  midiService.update();

  // First event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  // Should be bypass 0
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(SwitchId::kBypass));
  TEST_ASSERT_EQUAL(0, e.m_data.value);

  // Check there is no other event
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // CC message bypass on
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x04);
  midiHandler->pushByte(0x7F);

  midiService.update();

  // First event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  // Should be bypass 127
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(SwitchId::kBypass));
  TEST_ASSERT_EQUAL(127, e.m_data.value);

  // Check there is no other event
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_cc_invalid_data() {
  LogicalState logicalState;
  MockedClock clock;
  MidiService midiService(logicalState, clock);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  // ???
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x00);

  midiService.update();

  // Check no event
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // CC message POT2 32
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x02);
  midiHandler->pushByte(0x20);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  // Should be MIX at 32
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(PotId::kPot2));
  TEST_ASSERT_EQUAL(32, e.m_data.value);

  // Check no event
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // ???
  midiHandler->pushByte(0xB0);

  midiService.update();

  // Check no event
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // CC message MIX 32
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x03);
  midiHandler->pushByte(0x20);

  midiService.update();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  // Should be MIX at 32
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(PotId::kMixPot));
  TEST_ASSERT_EQUAL(32, e.m_data.value);

  // Check there is no other event
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_basic_pc_message() {
  LogicalState logicalState;
  MockedClock clock;
  MidiService midiService(logicalState, clock);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  // PC Message program 3
  midiHandler->pushByte(0xC0);
  midiHandler->pushByte(0x03);

  midiService.update();

  // CHeck event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_EQUAL(3, e.m_data.value);

  // Check there is no other event
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_pc_invalid_data() {
  LogicalState logicalState;
  MockedClock clock;
  MidiService midiService(logicalState, clock);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  midiService.init();

  midiHandler->pushByte(0xC0);
  midiHandler->pushByte(0x81);

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_bypass() {
  LogicalState logicalState;
  MockedClock clock;
  MidiService midiService(logicalState, clock);
  MidiHandler* midiHandler = midiService.getMidiHandler();

  // CC message bypass on
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x04);
  midiHandler->pushByte(0x7F);

  midiService.update();

  // CHeck event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  // Should be bypass 127
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(SwitchId::kBypass));
  TEST_ASSERT_EQUAL(MidiCCValues::c_bypassEnable, e.m_data.value);

  // Check there is no other event
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // CC message bypass off
  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x04);
  midiHandler->pushByte(0x00);

  midiService.update();

  // CHeck event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  // Should be bypass 0
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(SwitchId::kBypass));
  TEST_ASSERT_EQUAL(MidiCCValues::c_bypassDisable, e.m_data.value);

  // Check there is no other event
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_basic_cc_message);
  RUN_TEST(test_successive_cc_messages);
  RUN_TEST(test_cc_invalid_data);
  RUN_TEST(test_basic_pc_message);
  RUN_TEST(test_pc_invalid_data);
  RUN_TEST(test_bypass);
  UNITY_END();
}
