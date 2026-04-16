#include <unity.h>
#include "../interaction_fixture.h"
#include "../src/logic/programs.h"
#include "../src/logic/crossfade_handler.cpp"
#include "../src/logic/expr_handler.cpp"
#include "../src/logic/fv1_handler.cpp"
#include "../src/logic/memory_handler.cpp"
#include "../src/logic/menu_handler.cpp"
#include "../src/logic/midi_handler.cpp"
#include "../src/logic/pot_handler.cpp"
#include "../src/logic/preset_handler.cpp"
#include "../src/logic/tempo_handler.cpp"
#include "../src/logic/tap_handler.cpp"
#include "../src/services/fsm_service.cpp"
#include "../src/services/midi_service.cpp"
#include "../src/services/settings_service.cpp"
#include "../src/services/preset_bank_service.cpp"
#include "../src/services/preset_service.cpp"
#include "../src/services/program_mode_service.cpp"
#include "../src/services/program_service.cpp"
#include "../src/services/bypass_service.cpp"
#include "../src/services/expr_service.cpp"
#include "../src/services/pot_service.cpp"
#include "../src/services/tap_service.cpp"
#include "../src/services/tempo_service.cpp"
#include "../src/services/fv1_service.cpp"
#include "../src/services/crossfade_service.cpp"
#include "../src/services/menu_service.cpp"
#include "../src/services/display_service.cpp"
#include "../src/ui/menu_model.cpp"

// =============================================================================
// Test Helpers
// =============================================================================

Event makeBootEvent() {
  Event e{};
  e.m_domain = EventDomain::kSystem;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kBooted;
  return e;
}

Event makeDriverSwitchPress(SwitchId t_id, uint32_t t_timestamp = 0) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeDriverSwitchLongPress(SwitchId t_id, uint32_t t_timestamp = 0) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeDriverPotMove(PotId t_id, uint16_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.value = t_value;
  return e;
}

Event makeDriverExprMove(uint16_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

Event makeDriverEncoderDelta(int16_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kEncoder;
  e.m_action = EventAction::kDeltaChanged;
  e.m_data.delta = t_delta;
  return e;
}

bool eventWasRepublishedAsPhysical(InteractionFixture& fix, const Event& t_original) {
  Event e;
  while (fix.recallEvent(e)) {
    if (e.m_domain == EventDomain::kPhysical
        && e.m_subject == t_original.m_subject
        && e.m_action == t_original.m_action
        && e.m_id == t_original.m_id) {
      return true;
    }
  }
  return false;
}

void setUp() {}
void tearDown() {}

// =============================================================================
// kBoot / kRestoreState - All driver events filtered
// =============================================================================

void test_restore_state_filters_bypass_press() {
  InteractionFixture fix;
  fix.init();
  // FSM starts in kRestoreState after init()
  TEST_ASSERT_EQUAL(AppState::kRestoreState, fix.fsmService.getAppState());

  fix.clearEventBus();
  Event driverEvent = makeDriverSwitchPress(SwitchId::kBypass);
  fix.publishAndDispatchEvent(driverEvent);

  // No physical event should be published
  TEST_ASSERT_FALSE(fix.hasEvents());
}

void test_restore_state_filters_tap_press() {
  InteractionFixture fix;
  fix.init();
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(fix.hasEvents());
}

void test_restore_state_filters_pot_move() {
  InteractionFixture fix;
  fix.init();
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot0, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(fix.hasEvents());
}

void test_restore_state_filters_expr_move() {
  InteractionFixture fix;
  fix.init();
  fix.clearEventBus();

  Event driverEvent = makeDriverExprMove(512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(fix.hasEvents());
}

void test_restore_state_filters_encoder_delta() {
  InteractionFixture fix;
  fix.init();
  fix.clearEventBus();

  Event driverEvent = makeDriverEncoderDelta(1);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(fix.hasEvents());
}

void test_restore_state_filters_encoder_long_press() {
  InteractionFixture fix;
  fix.init();
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(fix.hasEvents());
}

// =============================================================================
// kBypassed - Only bypass switch press republished
// =============================================================================

void test_bypassed_republishes_bypass_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  TEST_ASSERT_EQUAL(AppState::kBypassed, fix.fsmService.getAppState());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kBypass);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_bypassed_filters_tap_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_bypassed_filters_tap_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_bypassed_filters_encoder_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_bypassed_filters_encoder_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_bypassed_filters_encoder_delta() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverEncoderDelta(1);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_bypassed_filters_program_mode_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kProgramMode);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_bypassed_filters_pot_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot0, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_bypassed_filters_expr_move() {
  InteractionFixture fix;
  fix.init();
  fix.clearEventBus();

  Event driverEvent = makeDriverExprMove(512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(fix.hasEvents());
}

// =============================================================================
// kProgramIdle - Selective republishing
// =============================================================================

void test_program_idle_republishes_bypass_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fix.fsmService.getAppState());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kBypass);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_idle_republishes_tap_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_idle_republishes_tap_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_idle_republishes_program_mode_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kProgramMode);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_idle_republishes_expr_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.clearEventBus();

  Event driverEvent = makeDriverExprMove(512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_idle_republishes_menu_lock_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuLock);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_idle_filters_encoder_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_idle_filters_encoder_delta() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.clearEventBus();

  Event driverEvent = makeDriverEncoderDelta(1);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_idle_filters_pot_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot0, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_idle_filters_encoder_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

// =============================================================================
// kProgramEdit - Most events republished
// =============================================================================

void test_program_edit_republishes_bypass_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kBypass);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_encoder_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_encoder_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_encoder_delta() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverEncoderDelta(1);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_tap_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_tap_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_pot_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot0, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_pot1_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot1, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_pot2_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot2, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_expr_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverExprMove(512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_program_mode_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kProgramMode);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_program_edit_republishes_menu_lock_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kProgramEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuLock);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

// =============================================================================
// kPresetIdle - Limited republishing
// =============================================================================

void test_preset_idle_republishes_bypass_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fix.fsmService.getAppState());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kBypass);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_idle_republishes_program_mode_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kProgramMode);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_idle_republishes_tap_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_idle_republishes_tap_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_idle_republishes_expr_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverExprMove(512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_idle_republishes_menu_lock_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuLock);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_idle_filters_encoder_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_idle_filters_encoder_delta() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverEncoderDelta(1);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_idle_filters_pot_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot0, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_idle_filters_encoder_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  // Encoder long press should be filtered in preset idle (no menu editing)
  TEST_ASSERT_FALSE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

// =============================================================================
// kPresetEdit - Most events republished (mirrors kProgramEdit)
// =============================================================================

void test_preset_edit_republishes_bypass_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kBypass);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_encoder_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_encoder_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuEncoder);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_encoder_delta() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverEncoderDelta(1);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_tap_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_tap_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kTap);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_pot_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot0, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_pot1_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot1, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_pot2_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverPotMove(PotId::kPot2, 512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_expr_move() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverExprMove(512);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_menu_lock_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kMenuLock);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

void test_preset_edit_republishes_program_mode_long_press() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();
  fix.publishAndDispatchAllEvents(makeBootEvent());
  fix.fsmService.setAppState(AppState::kPresetEdit);
  fix.clearEventBus();

  Event driverEvent = makeDriverSwitchLongPress(SwitchId::kProgramMode);
  fix.publishAndDispatchEvent(driverEvent);

  TEST_ASSERT_TRUE(eventWasRepublishedAsPhysical(fix, driverEvent));
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // kBoot / kRestoreState - All driver events filtered
  RUN_TEST(test_restore_state_filters_bypass_press);
  RUN_TEST(test_restore_state_filters_tap_press);
  RUN_TEST(test_restore_state_filters_pot_move);
  RUN_TEST(test_restore_state_filters_encoder_delta);
  RUN_TEST(test_restore_state_filters_encoder_long_press);
  RUN_TEST(test_restore_state_filters_expr_move);

  // kBypassed - Only bypass switch press republished
  RUN_TEST(test_bypassed_republishes_bypass_press);
  RUN_TEST(test_bypassed_filters_tap_press);
  RUN_TEST(test_bypassed_filters_tap_long_press);
  RUN_TEST(test_bypassed_filters_encoder_press);
  RUN_TEST(test_bypassed_filters_encoder_long_press);
  RUN_TEST(test_bypassed_filters_encoder_delta);
  RUN_TEST(test_bypassed_filters_program_mode_long_press);
  RUN_TEST(test_bypassed_filters_pot_move);
  RUN_TEST(test_bypassed_filters_expr_move);

  // kProgramIdle - Selective republishing
  RUN_TEST(test_program_idle_republishes_bypass_press);
  RUN_TEST(test_program_idle_republishes_tap_press);
  RUN_TEST(test_program_idle_republishes_tap_long_press);
  RUN_TEST(test_program_idle_republishes_program_mode_long_press);
  RUN_TEST(test_program_idle_republishes_expr_move);
  RUN_TEST(test_program_idle_republishes_menu_lock_long_press);
  RUN_TEST(test_program_idle_filters_encoder_press);
  RUN_TEST(test_program_idle_filters_encoder_long_press);
  RUN_TEST(test_program_idle_filters_encoder_delta);
  RUN_TEST(test_program_idle_filters_pot_move);

  // kProgramEdit - Most events republished
  RUN_TEST(test_program_edit_republishes_bypass_press);
  RUN_TEST(test_program_edit_republishes_encoder_press);
  RUN_TEST(test_program_edit_republishes_encoder_long_press);
  RUN_TEST(test_program_edit_republishes_encoder_delta);
  RUN_TEST(test_program_edit_republishes_tap_press);
  RUN_TEST(test_program_edit_republishes_tap_long_press);
  RUN_TEST(test_program_edit_republishes_pot_move);
  RUN_TEST(test_program_edit_republishes_pot1_move);
  RUN_TEST(test_program_edit_republishes_pot2_move);
  RUN_TEST(test_program_edit_republishes_expr_move);
  RUN_TEST(test_program_edit_republishes_menu_lock_long_press);
  RUN_TEST(test_program_edit_republishes_program_mode_long_press);

  // kPresetIdle - Limited republishing
  RUN_TEST(test_preset_idle_republishes_bypass_press);
  RUN_TEST(test_preset_idle_republishes_program_mode_long_press);
  RUN_TEST(test_preset_idle_republishes_tap_press);
  RUN_TEST(test_preset_idle_republishes_tap_long_press);
  RUN_TEST(test_preset_idle_republishes_expr_move);
  RUN_TEST(test_preset_idle_republishes_menu_lock_long_press);
  RUN_TEST(test_preset_idle_filters_encoder_press);
  RUN_TEST(test_preset_idle_filters_encoder_delta);
  RUN_TEST(test_preset_idle_filters_pot_move);
  RUN_TEST(test_preset_idle_filters_encoder_long_press);

  // kPresetEdit - Most events republished (mirrors kProgramEdit)
  RUN_TEST(test_preset_edit_republishes_bypass_press);
  RUN_TEST(test_preset_edit_republishes_encoder_press);
  RUN_TEST(test_preset_edit_republishes_encoder_long_press);
  RUN_TEST(test_preset_edit_republishes_encoder_delta);
  RUN_TEST(test_preset_edit_republishes_tap_press);
  RUN_TEST(test_preset_edit_republishes_tap_long_press);
  RUN_TEST(test_preset_edit_republishes_pot_move);
  RUN_TEST(test_preset_edit_republishes_pot1_move);
  RUN_TEST(test_preset_edit_republishes_pot2_move);
  RUN_TEST(test_preset_edit_republishes_expr_move);
  RUN_TEST(test_preset_edit_republishes_menu_lock_long_press);
  RUN_TEST(test_preset_edit_republishes_program_mode_long_press);

  return UNITY_END();
}
