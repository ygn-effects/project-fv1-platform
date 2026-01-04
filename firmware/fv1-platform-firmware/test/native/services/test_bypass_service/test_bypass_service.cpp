#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/bypass_service.h"
#include "mock/mock_bypass.h"

#include "../src/services/bypass_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_init_when_active() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Default state is kActive
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  bypassService.init();

  // Bypass relay should be on (effects active)
  TEST_ASSERT_EQUAL(1, mockBypass.m_kState);
}

void test_init_when_bypassed() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Set state to bypassed before init
  logicalState.m_bypassState = BypassState::kBypassed;

  bypassService.init();

  // Bypass relay should be off (effects bypassed)
  TEST_ASSERT_EQUAL(0, mockBypass.m_kState);
}

void test_bypass() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  // Send a footswitch pressed event
  Event e {EventDomain::kPhysical, EventSubject::kSwitch, EventAction::kPressed, 0, 0, {}};
  bypassService.handleEvent(e);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  // First event should be bypass state toggled
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kToggled, e.m_action);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  // Second event should be bypass state save
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);

  // Check LogicalState
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);

  // Check MockBypass state (toggled from initial 0)
  TEST_ASSERT_EQUAL(1, mockBypass.m_kState);

  // Send a footswitch pressed event
  e = {EventDomain::kPhysical, EventSubject::kSwitch, EventAction::kPressed, 0, 0, {}};
  bypassService.handleEvent(e);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  // First event should be bypass state toggled
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kToggled, e.m_action);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  // Second event should be bypass state save
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);

  // Check LogicalState
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  // Check MockBypass state (toggled back to 0)
  TEST_ASSERT_EQUAL(0, mockBypass.m_kState);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_midi_bypass_disable() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Start with active state
  logicalState.m_bypassState = BypassState::kActive;
  mockBypass.on();

  // Send MIDI bypass disable event (value = 0)
  Event e {EventDomain::kMidi, EventSubject::kSwitch, EventAction::kValueChanged, static_cast<uint8_t>(SwitchId::kBypass), 0, {}};
  e.m_data.value = MidiCCValues::c_bypassDisable;

  bypassService.handleEvent(e);

  // Check LogicalState changed to bypassed
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);

  // Check MockBypass is off
  TEST_ASSERT_EQUAL(0, mockBypass.m_kState);

  // Verify events published
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kToggled, e.m_action);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_midi_bypass_enable() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Start with bypassed state
  logicalState.m_bypassState = BypassState::kBypassed;
  mockBypass.off();

  // Send MIDI bypass enable event (value = 127)
  Event e {EventDomain::kMidi, EventSubject::kSwitch, EventAction::kValueChanged, static_cast<uint8_t>(SwitchId::kBypass), 0, {}};
  e.m_data.value = MidiCCValues::c_bypassEnable;

  bypassService.handleEvent(e);

  // Check LogicalState changed to active
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  // Check MockBypass is on
  TEST_ASSERT_EQUAL(1, mockBypass.m_kState);

  // Verify events published
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kToggled, e.m_action);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_interested_in() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Bypass footswitch press
  Event e {EventDomain::kPhysical, EventSubject::kSwitch, EventAction::kPressed, static_cast<u_int8_t>(SwitchId::kBypass), 0, {}};
  TEST_ASSERT_TRUE(bypassService.interestedIn(e));

  // MIDI switch press
  e = {EventDomain::kMidi, EventSubject::kSwitch, EventAction::kValueChanged, static_cast<u_int8_t>(SwitchId::kBypass), 0, {}};
  TEST_ASSERT_TRUE(bypassService.interestedIn(e));

  // Bypass footswitch longpress
  e = {EventDomain::kPhysical, EventSubject::kSwitch, EventAction::kLongPressed, static_cast<u_int8_t>(SwitchId::kBypass), 0, {}};
  TEST_ASSERT_FALSE(bypassService.interestedIn(e));

  // Random switch press
  e = {EventDomain::kPhysical, EventSubject::kSwitch, EventAction::kPressed, static_cast<u_int8_t>(SwitchId::kTap), 0, {}};
  TEST_ASSERT_FALSE(bypassService.interestedIn(e));

  // Pot move
  e = {EventDomain::kPhysical, EventSubject::kPot, EventAction::kValueChanged, static_cast<u_int8_t>(SwitchId::kTap), 0, {}};
  TEST_ASSERT_FALSE(bypassService.interestedIn(e));

  // Program change
  e = {EventDomain::kLogic, EventSubject::kProgram, EventAction::kValueChanged, static_cast<u_int8_t>(SwitchId::kTap), 0, {}};
  TEST_ASSERT_FALSE(bypassService.interestedIn(e));

  // Nonsensical event
  e = {EventDomain::kSystem, EventSubject::kTempo, EventAction::kPressed, static_cast<u_int8_t>(SwitchId::kTap), 0, {}};
  TEST_ASSERT_FALSE(bypassService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_init_when_active);
  RUN_TEST(test_init_when_bypassed);
  RUN_TEST(test_bypass);
  RUN_TEST(test_midi_bypass_disable);
  RUN_TEST(test_midi_bypass_enable);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
