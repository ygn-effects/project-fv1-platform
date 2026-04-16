#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/fsm_service.h"

#include "../src/services/fsm_service.cpp"

// =============================================================================
// Helper Functions
// =============================================================================

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

Event makeBootedEvent() {
  Event e{};
  e.m_domain = EventDomain::kSystem;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kBooted;
  return e;
}

Event makeBypassToggledEvent() {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kBypass;
  e.m_action = EventAction::kToggled;
  return e;
}

Event makeMenuLockedEvent() {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kLocked;
  return e;
}

Event makeMenuUnlockedEvent() {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kUnlocked;
  return e;
}

Event makeProgramModeToggledEvent() {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;
  return e;
}

void assertNoMoreEvents() {
  TEST_ASSERT_FALSE_MESSAGE(EventBus::hasEvent(), "Unexpected event on bus");
}

void setUp() {
  clearEventBus();
}

void tearDown() {}

// =============================================================================
// Init
// =============================================================================

void test_init_transitions_to_restore_state() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  TEST_ASSERT_EQUAL(AppState::kBoot, fsmService.getAppState());

  fsmService.init();

  TEST_ASSERT_EQUAL(AppState::kRestoreState, fsmService.getAppState());
  assertNoMoreEvents();
}

// =============================================================================
// Boot Transitions
// =============================================================================

void test_boot_active_program_mode_transitions_to_program_edit() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_programMode = ProgramMode::kProgram;

  fsmService.handleEvent(makeBootedEvent());

  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_boot_active_preset_mode_transitions_to_preset_idle() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_programMode = ProgramMode::kPreset;

  fsmService.handleEvent(makeBootedEvent());

  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_boot_bypassed_transitions_to_bypassed() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  logicalState.m_bypassState = BypassState::kBypassed;

  fsmService.handleEvent(makeBootedEvent());

  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());
  assertNoMoreEvents();
}

// =============================================================================
// Bypass Toggle State Transitions
// =============================================================================

void test_bypass_toggle_bypassed_to_program_edit() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  logicalState.m_programMode = ProgramMode::kProgram;
  fsmService.setAppState(AppState::kBypassed);

  fsmService.handleEvent(makeBypassToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_bypass_toggle_bypassed_to_preset_idle() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  logicalState.m_programMode = ProgramMode::kPreset;
  fsmService.setAppState(AppState::kBypassed);

  fsmService.handleEvent(makeBypassToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_bypass_toggle_program_edit_to_bypassed() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramEdit);

  fsmService.handleEvent(makeBypassToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_bypass_toggle_program_idle_to_bypassed() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramIdle);

  fsmService.handleEvent(makeBypassToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_bypass_toggle_preset_idle_to_bypassed() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kPresetIdle);

  fsmService.handleEvent(makeBypassToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_bypass_toggle_preset_edit_to_bypassed() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kPresetEdit);

  fsmService.handleEvent(makeBypassToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());
  assertNoMoreEvents();
}

// =============================================================================
// Menu Lock/Unlock State Transitions
// =============================================================================

void test_menu_unlock_program_idle_to_program_edit() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramIdle);

  fsmService.handleEvent(makeMenuUnlockedEvent());

  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_menu_lock_program_edit_to_program_idle() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramEdit);

  fsmService.handleEvent(makeMenuLockedEvent());

  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_menu_unlock_preset_idle_to_preset_edit() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kPresetIdle);

  fsmService.handleEvent(makeMenuUnlockedEvent());

  TEST_ASSERT_EQUAL(AppState::kPresetEdit, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_menu_lock_preset_edit_to_preset_idle() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kPresetEdit);

  fsmService.handleEvent(makeMenuLockedEvent());

  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_menu_unlock_ignored_in_bypassed() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kBypassed);

  fsmService.handleEvent(makeMenuUnlockedEvent());

  TEST_ASSERT_EQUAL(AppState::kBypassed, fsmService.getAppState());
  assertNoMoreEvents();
}

// =============================================================================
// Program Mode Toggle State Transitions
// =============================================================================

void test_program_mode_toggle_program_idle_to_preset_idle() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramIdle);

  fsmService.handleEvent(makeProgramModeToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_program_mode_toggle_program_edit_to_preset_idle() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kProgramEdit);

  fsmService.handleEvent(makeProgramModeToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_program_mode_toggle_preset_idle_to_program_edit() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kPresetIdle);

  fsmService.handleEvent(makeProgramModeToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
  assertNoMoreEvents();
}

void test_program_mode_toggle_preset_edit_to_program_edit() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.setAppState(AppState::kPresetEdit);

  fsmService.handleEvent(makeProgramModeToggledEvent());

  TEST_ASSERT_EQUAL(AppState::kProgramEdit, fsmService.getAppState());
  assertNoMoreEvents();
}

// =============================================================================
// interestedIn
// =============================================================================

void test_interested_in_system_booted() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event e{EventDomain::kSystem, EventSubject::kGeneral, EventAction::kBooted, 0, 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(e));
}

void test_interested_in_driver_events() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event e{EventDomain::kDriver, EventSubject::kSwitch, EventAction::kPressed, 0, 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(e));
}

void test_interested_in_ui_lock_unlock() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event lock{EventDomain::kUI, EventSubject::kMenu, EventAction::kLocked, 0, 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(lock));

  Event unlock{EventDomain::kUI, EventSubject::kMenu, EventAction::kUnlocked, 0, 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(unlock));
}

void test_interested_in_logic_bypass_toggled() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event e{EventDomain::kLogic, EventSubject::kBypass, EventAction::kToggled, 0, 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(e));
}

void test_interested_in_logic_program_mode_toggled() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event e{EventDomain::kLogic, EventSubject::kProgramMode, EventAction::kToggled, 0, 0, {}};
  TEST_ASSERT_TRUE(fsmService.interestedIn(e));
}

void test_not_interested_in_unrelated_events() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event e{EventDomain::kSystem, EventSubject::kTempo, EventAction::kPressed, 0, 0, {}};
  TEST_ASSERT_FALSE(fsmService.interestedIn(e));

  Event e2{EventDomain::kMemory, EventSubject::kBypass, EventAction::kSave, 0, 0, {}};
  TEST_ASSERT_FALSE(fsmService.interestedIn(e2));

  Event e3{EventDomain::kPhysical, EventSubject::kSwitch, EventAction::kPressed, 0, 0, {}};
  TEST_ASSERT_FALSE(fsmService.interestedIn(e3));
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_transitions_to_restore_state);

  // Boot Transitions
  RUN_TEST(test_boot_active_program_mode_transitions_to_program_edit);
  RUN_TEST(test_boot_active_preset_mode_transitions_to_preset_idle);
  RUN_TEST(test_boot_bypassed_transitions_to_bypassed);

  // Bypass Toggle
  RUN_TEST(test_bypass_toggle_bypassed_to_program_edit);
  RUN_TEST(test_bypass_toggle_bypassed_to_preset_idle);
  RUN_TEST(test_bypass_toggle_program_edit_to_bypassed);
  RUN_TEST(test_bypass_toggle_program_idle_to_bypassed);
  RUN_TEST(test_bypass_toggle_preset_idle_to_bypassed);
  RUN_TEST(test_bypass_toggle_preset_edit_to_bypassed);

  // Menu Lock/Unlock
  RUN_TEST(test_menu_unlock_program_idle_to_program_edit);
  RUN_TEST(test_menu_lock_program_edit_to_program_idle);
  RUN_TEST(test_menu_unlock_preset_idle_to_preset_edit);
  RUN_TEST(test_menu_lock_preset_edit_to_preset_idle);
  RUN_TEST(test_menu_unlock_ignored_in_bypassed);

  // Program Mode Toggle
  RUN_TEST(test_program_mode_toggle_program_idle_to_preset_idle);
  RUN_TEST(test_program_mode_toggle_program_edit_to_preset_idle);
  RUN_TEST(test_program_mode_toggle_preset_idle_to_program_edit);
  RUN_TEST(test_program_mode_toggle_preset_edit_to_program_edit);

  // interestedIn
  RUN_TEST(test_interested_in_system_booted);
  RUN_TEST(test_interested_in_driver_events);
  RUN_TEST(test_interested_in_ui_lock_unlock);
  RUN_TEST(test_interested_in_logic_bypass_toggled);
  RUN_TEST(test_interested_in_logic_program_mode_toggled);
  RUN_TEST(test_not_interested_in_unrelated_events);

  UNITY_END();
}
