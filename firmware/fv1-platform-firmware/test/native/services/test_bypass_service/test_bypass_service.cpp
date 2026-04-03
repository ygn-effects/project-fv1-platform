#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/bypass_service.h"
#include "mock/mock_bypass.h"

#include "../src/services/bypass_service.cpp"

// =============================================================================
// Helper functions
// =============================================================================

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

Event makePhysicalBypassPressEvent() {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kBypass);
  return e;
}

Event makeMidiBypassEvent(uint16_t t_value) {
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(SwitchId::kBypass);
  e.m_data.value = t_value;
  return e;
}

void assertBypassToggledEventPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kToggled, e.m_action);
}

void assertBypassSaveEventPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
}

void setUp() {
  clearEventBus();
}

void tearDown() {

}

// =============================================================================
// Init Tests
// =============================================================================

void test_init_when_active() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Set active
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  bypassService.init();

  // Bypass relay should be on
  TEST_ASSERT_EQUAL(1, mockBypass.m_kState);
}

void test_init_when_bypassed() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Set bypassed
  logicalState.m_bypassState = BypassState::kBypassed;
  bypassService.init();

  // Bypass relay should be off
  TEST_ASSERT_EQUAL(0, mockBypass.m_kState);
}

// =============================================================================
// Physical Switch Tests
// =============================================================================

void test_physical_press_toggles_active_to_bypassed() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Start active
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  mockBypass.on();

  bypassService.handleEvent(makePhysicalBypassPressEvent());

  // State should toggle to bypassed
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(0, mockBypass.m_kState);

  // Verify events published
  assertBypassToggledEventPublished();
  assertBypassSaveEventPublished();
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_physical_press_toggles_bypassed_to_active() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Start bypassed
  logicalState.m_bypassState = BypassState::kBypassed;
  mockBypass.off();

  bypassService.handleEvent(makePhysicalBypassPressEvent());

  // State should toggle to active
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(1, mockBypass.m_kState);

  // Verify events published
  assertBypassToggledEventPublished();
  assertBypassSaveEventPublished();
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_physical_press_multiple_toggles() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // First toggle: active -> bypassed
  bypassService.handleEvent(makePhysicalBypassPressEvent());
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  clearEventBus();

  // Second toggle: bypassed -> active
  bypassService.handleEvent(makePhysicalBypassPressEvent());
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  clearEventBus();

  // Third toggle: active -> bypassed
  bypassService.handleEvent(makePhysicalBypassPressEvent());
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
}

// =============================================================================
// MIDI Tests
// =============================================================================

void test_midi_disable_when_active() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Start active
  logicalState.m_bypassState = BypassState::kActive;
  mockBypass.on();

  bypassService.handleEvent(makeMidiBypassEvent(MidiCCValues::c_bypassDisable));

  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(0, mockBypass.m_kState);

  // Verify events published
  assertBypassToggledEventPublished();
  assertBypassSaveEventPublished();
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_midi_enable_when_active() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Start active
  logicalState.m_bypassState = BypassState::kActive;
  mockBypass.on();

  bypassService.handleEvent(makeMidiBypassEvent(MidiCCValues::c_bypassEnable));

  // State shouldn't have changed
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(1, mockBypass.m_kState);

  // Verify no events published
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_midi_enable_when_bypassed() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Start bypassed
  logicalState.m_bypassState = BypassState::kBypassed;
  mockBypass.off();

  bypassService.handleEvent(makeMidiBypassEvent(MidiCCValues::c_bypassEnable));

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(1, mockBypass.m_kState);

  assertBypassToggledEventPublished();
  assertBypassSaveEventPublished();
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_midi_disable_when_bypassed() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  // Start active
  logicalState.m_bypassState = BypassState::kBypassed;
  mockBypass.off();

  bypassService.handleEvent(makeMidiBypassEvent(MidiCCValues::c_bypassDisable));

  // State shouldn't have changed
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(0, mockBypass.m_kState);

  // Verify no events published
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_physical_bypass_press() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kBypass);

  TEST_ASSERT_TRUE(bypassService.interestedIn(e));
}

void test_interested_in_midi_bypass() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(SwitchId::kBypass);

  TEST_ASSERT_TRUE(bypassService.interestedIn(e));
}

void test_not_interested_in_bypass_long_press() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kBypass);

  TEST_ASSERT_FALSE(bypassService.interestedIn(e));
}

void test_not_interested_in_other_switches() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);

  TEST_ASSERT_FALSE(bypassService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  Event e;

  // Not interested in pot events
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  TEST_ASSERT_FALSE(bypassService.interestedIn(e));

  // Not interested in program change
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  TEST_ASSERT_FALSE(bypassService.interestedIn(e));

  // Not interested in memory events
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kBypass;
  TEST_ASSERT_FALSE(bypassService.interestedIn(e));

  // Not interested in logic bypass events (outputs them)
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kBypass;
  TEST_ASSERT_FALSE(bypassService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_when_active);
  RUN_TEST(test_init_when_bypassed);

  // Physical Switch
  RUN_TEST(test_physical_press_toggles_active_to_bypassed);
  RUN_TEST(test_physical_press_toggles_bypassed_to_active);
  RUN_TEST(test_physical_press_multiple_toggles);

  // MIDI
  RUN_TEST(test_midi_disable_when_active);
  RUN_TEST(test_midi_enable_when_active);
  RUN_TEST(test_midi_enable_when_bypassed);
  RUN_TEST(test_midi_disable_when_bypassed);

  // interestedIn
  RUN_TEST(test_interested_in_physical_bypass_press);
  RUN_TEST(test_interested_in_midi_bypass);
  RUN_TEST(test_not_interested_in_bypass_long_press);
  RUN_TEST(test_not_interested_in_other_switches);
  RUN_TEST(test_not_interested_in_other_events);

  UNITY_END();
}
