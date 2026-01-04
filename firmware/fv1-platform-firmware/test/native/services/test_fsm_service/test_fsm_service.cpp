#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/fsm_service.h"

#include "../src/services/fsm_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_boot_transition() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event e;
  e.m_domain = EventDomain::kSystem;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kBooted;

  fsmService.handleEvent(e);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should be program idle
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fsmService.getAppState());
}

void test_boot_mode() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  logicalState.m_programMode = ProgramMode::kPreset;

  Event e;
  e.m_domain = EventDomain::kSystem;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kBooted;

  fsmService.handleEvent(e);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should be preset idle
  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fsmService.getAppState());
}

void test_switch_bypass() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event fsw;
  Event e;
  fsw.m_domain = EventDomain::kDriver;
  fsw.m_subject = EventSubject::kSwitch;
  fsw.m_action = EventAction::kPressed;
  fsw.m_id = static_cast<uint8_t>(SwitchId::kBypass);

  // Test1
  fsmService.setAppState(AppState::kBypassed);

  // Bypass press
  fsmService.handleEvent(fsw);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kBypass, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Test 2
  fsmService.setAppState(AppState::kProgramIdle);

  // Bypass press
  fsmService.handleEvent(fsw);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kBypass, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Test 3
  fsmService.setAppState(AppState::kPresetIdle);

  // Bypass press
  fsmService.handleEvent(fsw);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kBypass, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Test 4
  fsmService.setAppState(AppState::kProgramEdit);

  // Bypass press
  fsmService.handleEvent(fsw);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kBypass, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_bypass_toggle() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event byp;
  byp.m_domain = EventDomain::kLogic;
  byp.m_subject = EventSubject::kBypass;
  byp.m_action = EventAction::kToggled;

  // Test 1
  fsmService.setAppState(AppState::kBypassed);

  // Bypass toggle
  fsmService.handleEvent(byp);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Should be program idle
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fsmService.getAppState());

  // Bypass toggle
  fsmService.handleEvent(byp);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Should be bypassed
  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());

  // Test 2
  fsmService.setAppState(AppState::kProgramEdit);

  // Bypass toggle
  fsmService.handleEvent(byp);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Should be bypassed
  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());

  // Test 3
  fsmService.setAppState(AppState::kPresetIdle);

  // Bypass toggle
  fsmService.handleEvent(byp);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Should be bypassed
  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());
}

void test_menu_toggle() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event lck;
  lck.m_domain = EventDomain::kUI;
  lck.m_subject = EventSubject::kMenu;
  lck.m_action = EventAction::kLocked;
  Event unlck = lck;
  unlck.m_action = EventAction::kUnlocked;

  // Test 1
  fsmService.setAppState(AppState::kBypassed);

  // Bypass toggle
  fsmService.handleEvent(unlck);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Should remain bypassed (menu unlock ignored in bypass state)
  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());

  // Test 2
  fsmService.setAppState(AppState::kProgramIdle);

  // Menu unlock
  fsmService.handleEvent(unlck);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Should be program edit
  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());

  // Menu lock
  fsmService.handleEvent(lck);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Should be program idle
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fsmService.getAppState());
}

void test_program_mode_toggle() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event prgm;
  prgm.m_domain = EventDomain::kLogic;
  prgm.m_subject = EventSubject::kProgramMode;
  prgm.m_action = EventAction::kToggled;

  // Program idle mode
  fsmService.setAppState(AppState::kProgramIdle);

  // Program mode toggle event
  fsmService.handleEvent(prgm);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Should be preset mode idle
  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fsmService.getAppState());

  // Program mode toggle event
  fsmService.handleEvent(prgm);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Should be preset mode idle
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fsmService.getAppState());
}

void test_init() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  // Before init, state should be kBoot
  TEST_ASSERT_EQUAL(AppState::kBoot, fsmService.getAppState());

  fsmService.init();

  // After init, state should be kRestoreState
  TEST_ASSERT_EQUAL(AppState::kRestoreState, fsmService.getAppState());
}

void test_boot_to_bypassed() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  // Set bypass state to bypassed (effects off)
  logicalState.m_bypassState = BypassState::kBypassed;

  Event e;
  e.m_domain = EventDomain::kSystem;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kBooted;

  fsmService.handleEvent(e);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should be bypassed
  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());
}

void test_program_idle_tap_press() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramIdle);

  Event tap;
  tap.m_domain = EventDomain::kDriver;
  tap.m_subject = EventSubject::kSwitch;
  tap.m_action = EventAction::kPressed;
  tap.m_id = static_cast<uint8_t>(SwitchId::kTap);

  fsmService.handleEvent(tap);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kTap, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program idle
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fsmService.getAppState());
}

void test_program_idle_tap_long_press() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramIdle);

  Event tap;
  tap.m_domain = EventDomain::kDriver;
  tap.m_subject = EventSubject::kSwitch;
  tap.m_action = EventAction::kLongPressed;
  tap.m_id = static_cast<uint8_t>(SwitchId::kTap);

  fsmService.handleEvent(tap);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kLongPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kTap, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program idle
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fsmService.getAppState());
}

void test_program_idle_encoder_long_press() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramIdle);

  Event enc;
  enc.m_domain = EventDomain::kDriver;
  enc.m_subject = EventSubject::kSwitch;
  enc.m_action = EventAction::kLongPressed;
  enc.m_id = static_cast<uint8_t>(SwitchId::kMenuEncoder);

  fsmService.handleEvent(enc);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kLongPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kMenuEncoder, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program idle
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fsmService.getAppState());
}

void test_program_idle_program_mode_long_press() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramIdle);

  Event pm;
  pm.m_domain = EventDomain::kDriver;
  pm.m_subject = EventSubject::kSwitch;
  pm.m_action = EventAction::kLongPressed;
  pm.m_id = static_cast<uint8_t>(SwitchId::kProgramMode);

  fsmService.handleEvent(pm);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kLongPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kProgramMode, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program idle
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fsmService.getAppState());
}

void test_program_edit_encoder_press() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramEdit);

  Event enc;
  enc.m_domain = EventDomain::kDriver;
  enc.m_subject = EventSubject::kSwitch;
  enc.m_action = EventAction::kPressed;
  enc.m_id = static_cast<uint8_t>(SwitchId::kMenuEncoder);

  fsmService.handleEvent(enc);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kMenuEncoder, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program edit
  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
}

void test_program_edit_encoder_long_press() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramEdit);

  Event enc;
  enc.m_domain = EventDomain::kDriver;
  enc.m_subject = EventSubject::kSwitch;
  enc.m_action = EventAction::kLongPressed;
  enc.m_id = static_cast<uint8_t>(SwitchId::kMenuEncoder);

  fsmService.handleEvent(enc);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kLongPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kMenuEncoder, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program edit
  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
}

void test_program_edit_encoder_delta_changed() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramEdit);

  Event enc;
  enc.m_domain = EventDomain::kDriver;
  enc.m_subject = EventSubject::kEncoder;
  enc.m_action = EventAction::kDeltaChanged;
  enc.m_id = static_cast<uint8_t>(SwitchId::kMenuEncoder);

  fsmService.handleEvent(enc);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kEncoder, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kDeltaChanged, e.m_action);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program edit
  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
}

void test_program_edit_tap_press() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramEdit);

  Event tap;
  tap.m_domain = EventDomain::kDriver;
  tap.m_subject = EventSubject::kSwitch;
  tap.m_action = EventAction::kPressed;
  tap.m_id = static_cast<uint8_t>(SwitchId::kTap);

  fsmService.handleEvent(tap);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kTap, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program edit
  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
}

void test_program_edit_tap_long_press() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramEdit);

  Event tap;
  tap.m_domain = EventDomain::kDriver;
  tap.m_subject = EventSubject::kSwitch;
  tap.m_action = EventAction::kLongPressed;
  tap.m_id = static_cast<uint8_t>(SwitchId::kTap);

  fsmService.handleEvent(tap);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kLongPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kTap, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program edit
  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
}

void test_program_edit_pot_value_changed() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramEdit);

  Event pot;
  pot.m_domain = EventDomain::kDriver;
  pot.m_subject = EventSubject::kPot;
  pot.m_action = EventAction::kValueChanged;
  pot.m_id = 0;

  fsmService.handleEvent(pot);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain program edit
  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
}

void test_preset_idle_program_mode_long_press() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kPresetIdle);

  Event pm;
  pm.m_domain = EventDomain::kDriver;
  pm.m_subject = EventSubject::kSwitch;
  pm.m_action = EventAction::kLongPressed;
  pm.m_id = static_cast<uint8_t>(SwitchId::kProgramMode);

  fsmService.handleEvent(pm);

  // Event bus should hold the republished driver event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kLongPressed, e.m_action);
  TEST_ASSERT_EQUAL(SwitchId::kProgramMode, static_cast<SwitchId>(e.m_id));

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // State should remain preset idle
  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fsmService.getAppState());
}

void test_interested_in() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  // Boot complete event
  Event e {EventDomain::kSystem, EventSubject::kGeneral, EventAction::kBooted, static_cast<u_int8_t>(SwitchId::kBypass), 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(e));

  // Bypass footswitch press
  e = {EventDomain::kDriver, EventSubject::kSwitch, EventAction::kPressed, static_cast<u_int8_t>(SwitchId::kBypass), 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(e));

  // UI domain lock/unlock events
  e = {EventDomain::kUI, EventSubject::kMenu, EventAction::kLocked, 0, 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(e));

  e = {EventDomain::kUI, EventSubject::kMenu, EventAction::kUnlocked, 0, 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(e));

  // Logic domain program mode toggle
  e = {EventDomain::kLogic, EventSubject::kProgramMode, EventAction::kToggled, 0, 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(e));

  // Nonsensical event
  e = {EventDomain::kSystem, EventSubject::kTempo, EventAction::kPressed, static_cast<u_int8_t>(SwitchId::kTap), 0, {}};
  TEST_ASSERT_FALSE(fsmService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_init);
  RUN_TEST(test_boot_transition);
  RUN_TEST(test_boot_mode);
  RUN_TEST(test_boot_to_bypassed);
  RUN_TEST(test_switch_bypass);
  RUN_TEST(test_bypass_toggle);
  RUN_TEST(test_menu_toggle);
  RUN_TEST(test_program_mode_toggle);
  RUN_TEST(test_program_idle_tap_press);
  RUN_TEST(test_program_idle_tap_long_press);
  RUN_TEST(test_program_idle_encoder_long_press);
  RUN_TEST(test_program_idle_program_mode_long_press);
  RUN_TEST(test_program_edit_encoder_press);
  RUN_TEST(test_program_edit_encoder_long_press);
  RUN_TEST(test_program_edit_encoder_delta_changed);
  RUN_TEST(test_program_edit_tap_press);
  RUN_TEST(test_program_edit_tap_long_press);
  RUN_TEST(test_program_edit_pot_value_changed);
  RUN_TEST(test_preset_idle_program_mode_long_press);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
