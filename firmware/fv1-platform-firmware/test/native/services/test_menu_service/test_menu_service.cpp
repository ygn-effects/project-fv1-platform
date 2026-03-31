#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/menu_service.h"
#include "ui/menu_model.h"
#include "mock/mock_clock.h"
#include "mock/mock_led.h"

#include "../src/services/menu_service.cpp"
#include "../src/logic/menu_handler.cpp"
#include "../src/ui/menu_model.cpp"

// =============================================================================
// Helper Functions
// =============================================================================

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

Event makePhysicalSwitchLongPressEvent(SwitchId t_id, uint32_t t_timestamp = 0) {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_timestamp = t_timestamp;
  return e;
}

Event makePhysicalSwitchPressEvent(SwitchId t_id, uint32_t t_timestamp = 0) {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_timestamp = t_timestamp;
  return e;
}

Event makePhysicalEncoderDeltaEvent(EncoderId t_id, int16_t t_delta, uint32_t t_timestamp = 0) {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kEncoder;
  e.m_action = EventAction::kDeltaChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.delta = t_delta;
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeLogicPotValueChangedEvent(PotId t_id, uint16_t t_value, uint32_t t_timestamp = 0) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.value = t_value;
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeLogicTempoValueChangedEvent(uint16_t t_value, uint32_t t_timestamp = 0) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeLogicBypassToggledEvent(uint32_t t_timestamp = 0) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kBypass;
  e.m_action = EventAction::kToggled;
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeLogicProgramChangedEvent(uint8_t t_programId) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_id = t_programId;
  return e;
}

Event makeUIPresetSettingChangeEvent(SavePresetParam t_param, int16_t t_delta = 0) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSettingChanged;
  e.m_id = static_cast<uint8_t>(t_param);
  e.m_data.delta = t_delta;
  return e;
}

void assertMenuLockedEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected kLocked event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kUI, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kMenu, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kLocked, e.m_action);
}

void assertMenuUnlockedEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected kUnlocked event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kUI, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kMenu, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kUnlocked, e.m_action);
}

void assertMenuUpdatedEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected kUpdated event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kUI, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kMenu, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kUpdated, e.m_action);
  TEST_ASSERT_NOT_NULL(e.m_data.ptr);
}

void assertNoMoreEvents() {
  TEST_ASSERT_FALSE_MESSAGE(EventBus::hasEvent(), "Unexpected event on bus");
}

void setUp() {
  clearEventBus();
}

void tearDown() {}

// =============================================================================
// Init Tests
// =============================================================================

void test_init_publishes_updated_event() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  logicalState.m_programMode = ProgramMode::kPreset;
  service.init();

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_init_starts_unlocked_program_mode() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Encoder input should be ignored when locked
  service.handleEvent(makePhysicalEncoderDeltaEvent(EncoderId::kMenuEncoder, 1));

  // Menu published event because the cursor moved
  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_init_starts_locked_preset_mode() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  logicalState.m_programMode = ProgramMode::kPreset;
  service.init();
  clearEventBus();

  // Encoder input should be ignored when locked
  service.handleEvent(makePhysicalEncoderDeltaEvent(EncoderId::kMenuEncoder, 1));

  // Menu locked so no events
  assertNoMoreEvents();
}

void test_init_syncs_save_bank_preset_to_logical_state() {
  LogicalState logicalState;
  logicalState.m_currentPresetBank = 2;
  logicalState.m_currentPreset = 1;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  TEST_ASSERT_EQUAL(2, logicalState.m_saveTargetBank);
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetPreset);
}

void test_init_initializes_led() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  TEST_ASSERT_TRUE(mockLed.initialized);
}

void test_init_active_sets_led_on() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  logicalState.m_bypassState = BypassState::kActive;
  service.init();
  clearEventBus();

  TEST_ASSERT_EQUAL(1, mockLed.m_pinState);
}

void test_init_bypass_sets_led_off() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  logicalState.m_bypassState = BypassState::kBypassed;
  service.init();
  clearEventBus();

  TEST_ASSERT_EQUAL(0, mockLed.m_pinState);
}

// =============================================================================
// Bypass Toggled
// =============================================================================

void test_logic_bypass_toggled_bypass_sets_led_off() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  logicalState.m_bypassState = BypassState::kActive;
  service.init();
  clearEventBus();

  logicalState.m_bypassState = BypassState::kBypassed;
  service.handleEvent(makeLogicBypassToggledEvent());
  clearEventBus();

  TEST_ASSERT_EQUAL(0, mockLed.m_pinState);
}

void test_logic_bypass_toggled_active_sets_led_on() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  logicalState.m_bypassState = BypassState::kBypassed;
  service.init();
  clearEventBus();

  logicalState.m_bypassState = BypassState::kActive;
  service.handleEvent(makeLogicBypassToggledEvent());
  clearEventBus();

  TEST_ASSERT_EQUAL(1, mockLed.m_pinState);
}

// =============================================================================
// Lock/Unlock Tests
// =============================================================================

void test_menu_lock_long_press_locks_menu() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 1000));

  assertMenuLockedEventPublished();
  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_menu_lock_long_press_unlocks_menu() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Lock first
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 1000));
  clearEventBus();

  // Unlock again
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 2000));

  assertMenuUnlockedEventPublished();
  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_bypass_toggled_locks_menu() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Bypass toggle should lock
  service.handleEvent(makeLogicBypassToggledEvent(2000));

  assertMenuLockedEventPublished();
  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_bypass_toggled_ignored_when_locked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Lock first
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 1000));
  clearEventBus();

  // Bypass toggle should lock
  service.handleEvent(makeLogicBypassToggledEvent(2000));

  assertNoMoreEvents();
}

void test_bypass_toggled_restored_when_unlocked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  // Start active
  logicalState.m_bypassState = BypassState::kActive;

  service.init();
  clearEventBus();

  // Program mode starts unlocked, bypass
  service.handleEvent(makeLogicBypassToggledEvent(2000));
  assertMenuLockedEventPublished();
  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();

  // Active
  service.handleEvent(makeLogicBypassToggledEvent(2000));
  assertMenuUnlockedEventPublished();
  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

// =============================================================================
// Navigation Tests (Selecting Mode)
// =============================================================================

void test_encoder_delta_publishes_updated_when_unlocked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Move cursor
  service.handleEvent(makePhysicalEncoderDeltaEvent(EncoderId::kMenuEncoder, 1, 2000));

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_encoder_press_publishes_updated_when_unlocked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Press to select (enters editing on first item which has onMove)
  service.handleEvent(makePhysicalSwitchPressEvent(SwitchId::kMenuEncoder, 1000));

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_encoder_ignored_when_locked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Lock
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 1000));
  clearEventBus();

  // Try to use encoder while locked
  service.handleEvent(makePhysicalEncoderDeltaEvent(EncoderId::kMenuEncoder, 1, 1000));
  service.handleEvent(makePhysicalSwitchPressEvent(SwitchId::kMenuEncoder, 2000));

  assertNoMoreEvents();
}

// =============================================================================
// Pot Overlay Tests
// =============================================================================

void test_pot_value_changed_publishes_updated_when_unlocked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  // Use non-delay effect so Pot1 overlay works
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  service.init();
  clearEventBus();

  // Move pot
  service.handleEvent(makeLogicPotValueChangedEvent(PotId::kPot1, 512, 2000));

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_pot_value_changed_publishes_updated_when_locked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  // Use non-delay effect so Pot1 overlay works
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  service.init();
  clearEventBus();

  // Lock
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 1000));
  clearEventBus();

  // Move pot
  service.handleEvent(makeLogicPotValueChangedEvent(PotId::kPot1, 512, 2000));

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

// =============================================================================
// Tempo Overlay Tests
// =============================================================================

void test_tempo_change_publishes_updated_when_unlocked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Tempo change
  service.handleEvent(makeLogicTempoValueChangedEvent(500, 2000));

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_tempo_change_publishes_updated_when_locked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Lock
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 1000));
  clearEventBus();

  // Tempo change
  service.handleEvent(makeLogicTempoValueChangedEvent(500, 2000));

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

// =============================================================================
// Preset Save Overlay Tests
// =============================================================================

void test_menu_encoder_long_press_publishes_updated_when_unlocked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Menu encoder long press
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuEncoder));

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_ui_preset_setting_changed_sets_logical_state_target_preset() {
  LogicalState logicalState;
  logicalState.m_currentPresetBank = 2;
  logicalState.m_currentPreset = 1;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Menu encoder long press
  service.handleEvent(makeUIPresetSettingChangeEvent(SavePresetParam::kTargetPreset, 1));

  TEST_ASSERT_EQUAL(2, logicalState.m_saveTargetPreset);

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_ui_preset_setting_changed_sets_logical_state_target_bank() {
  LogicalState logicalState;
  logicalState.m_currentPresetBank = 2;
  logicalState.m_currentPreset = 1;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Menu encoder long press
  service.handleEvent(makeUIPresetSettingChangeEvent(SavePresetParam::kTargetBank, 1));

  TEST_ASSERT_EQUAL(3, logicalState.m_saveTargetBank);

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

// =============================================================================
// Editing Mode Tests
// =============================================================================

void test_editing_encoder_delta_publishes_updated() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Press to enter editing (first item has onMove)
  service.handleEvent(makePhysicalSwitchPressEvent(SwitchId::kMenuEncoder, 2000));
  clearEventBus();

  // Move encoder in editing mode
  service.handleEvent(makePhysicalEncoderDeltaEvent(EncoderId::kMenuEncoder, 1, 3000));

  // Should publish UI event from onMove callback + updated event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  // First event is from the onMove callback (kUI, kProgram, kValueChanged)
  TEST_ASSERT_EQUAL(EventDomain::kUI, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_editing_encoder_press_exits_editing() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Enter editing
  service.handleEvent(makePhysicalSwitchPressEvent(SwitchId::kMenuEncoder, 2000));
  clearEventBus();

  // Press again to exit editing
  service.handleEvent(makePhysicalSwitchPressEvent(SwitchId::kMenuEncoder, 3000));

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

// =============================================================================
// Timeout Tests
// =============================================================================

void test_update_not_locks_menu_after_timeout_program_mode() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Set clock at 1000
  mockClock.setClock(1000);

  // Advance time past timeout (default is 10000ms)
  mockClock.setClock(1000 + ui::MenuConstants::c_menuTimeout + 1);
  service.update();

  assertNoMoreEvents();
}

void test_update_locks_menu_after_timeout_preset_mode() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  logicalState.m_programMode = ProgramMode::kPreset;
  service.init();
  clearEventBus();

  // Unlock at time 1000
  mockClock.setClock(1000);
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 1000));
  clearEventBus();

  // Advance time past timeout (default is 10000ms)
  mockClock.setClock(1000 + ui::MenuConstants::c_menuTimeout + 1);
  service.update();

  assertMenuLockedEventPublished();
  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_update_does_not_lock_before_timeout_preset_mode() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  logicalState.m_programMode = ProgramMode::kPreset;
  service.init();
  clearEventBus();

  // Unlock at time 1000
  mockClock.setClock(1000);
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 1000));
  clearEventBus();

  // Advance time but not past timeout
  mockClock.setClock(1000 + ui::MenuConstants::c_menuTimeout - 1);
  service.update();

  assertNoMoreEvents();
}

void test_update_pops_pot_overlay_after_timeout() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  service.init();
  clearEventBus();

  // Set clock at 1000
  mockClock.setClock(1000);

  // Push pot overlay at time 2000
  mockClock.setClock(2000);
  service.handleEvent(makeLogicPotValueChangedEvent(PotId::kPot1, 512, 2000));
  clearEventBus();

  // Advance time past pot overlay timeout
  mockClock.setClock(2000 + ui::MenuConstants::c_potMenuTimeout + 1);
  service.update();

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_update_pops_tempo_overlay_after_timeout() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Set clock at 1000
  mockClock.setClock(1000);

  // Push tempo overlay at time 2000
  mockClock.setClock(2000);
  service.handleEvent(makeLogicTempoValueChangedEvent(500, 2000));
  clearEventBus();

  // Advance time past tempo overlay timeout
  mockClock.setClock(2000 + ui::MenuConstants::c_tempoMenuTimeout + 1);
  service.update();

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_update_does_nothing_when_locked() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  // Lock
  service.handleEvent(makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock, 0));
  clearEventBus();

  // Stay locked, advance time
  mockClock.setClock(100000);
  service.update();

  assertNoMoreEvents();
}

// =============================================================================
// Program change
// =============================================================================

void test_program_change_publishes_updated() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  service.handleEvent(makeLogicProgramChangedEvent(1));

  assertMenuUpdatedEventPublished();
  assertNoMoreEvents();
}

void test_program_change_syncs_save_bank_preset_to_logical_state_preset_mode() {
  LogicalState logicalState;
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_currentPresetBank = 3;
  logicalState.m_currentPreset = 2;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  logicalState.m_currentPreset = 1;
  service.handleEvent(makeLogicProgramChangedEvent(1));

  TEST_ASSERT_EQUAL(3, logicalState.m_saveTargetBank);
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetPreset);
}

void test_program_change_not_syncs_save_bank_preset_to_logical_state_program_mode() {
  LogicalState logicalState;
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_currentPresetBank = 3;
  logicalState.m_currentPreset = 2;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  service.init();
  clearEventBus();

  service.handleEvent(makeLogicProgramChangedEvent(1));

  TEST_ASSERT_EQUAL(3, logicalState.m_saveTargetBank);
  TEST_ASSERT_EQUAL(2, logicalState.m_saveTargetPreset);
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_physical_switch_long_press_menu_lock() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e = makePhysicalSwitchLongPressEvent(SwitchId::kMenuLock);
  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_interested_in_physical_switch_press_menu_encoder() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e = makePhysicalSwitchPressEvent(SwitchId::kMenuEncoder);
  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_interested_in_physical_encoder_move_menu_encoder() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e = makePhysicalEncoderDeltaEvent(EncoderId::kMenuEncoder, 1);
  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_interested_in_logic_pot_value_changed() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e = makeLogicPotValueChangedEvent(PotId::kPot1, 512);
  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_interested_in_logic_tempo_value_changed() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e = makeLogicTempoValueChangedEvent(500);
  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_interested_in_logic_bypass_toggled() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e = makeLogicBypassToggledEvent();
  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_interested_in_logic_program_changed() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e = makeLogicProgramChangedEvent(0);
  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_interested_in_ui_preset_seting_changed() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e = makeUIPresetSettingChangeEvent(SavePresetParam::kTargetBank);
  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_not_interested_in_other_switches() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e = makePhysicalSwitchLongPressEvent(SwitchId::kBypass);
  TEST_ASSERT_FALSE(service.interestedIn(e));

  e = makePhysicalSwitchLongPressEvent(SwitchId::kTap);
  TEST_ASSERT_FALSE(service.interestedIn(e));
}

void test_not_interested_in_switch_press() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kMenuLock);
  TEST_ASSERT_FALSE(service.interestedIn(e));
}

void test_not_interested_in_other_encoders() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  // Use an invalid encoder ID (cast from a number that's not kMenuEncoder)
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kEncoder;
  e.m_action = EventAction::kDeltaChanged;
  e.m_id = 99;  // Invalid encoder ID
  e.m_data.delta = 1;
  TEST_ASSERT_FALSE(service.interestedIn(e));
}

void test_not_interested_in_events_it_publishes() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  // Service publishes to kUI/kMenu - should not listen to these
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kLocked;
  TEST_ASSERT_FALSE(service.interestedIn(e));

  e.m_action = EventAction::kUnlocked;
  TEST_ASSERT_FALSE(service.interestedIn(e));

  e.m_action = EventAction::kUpdated;
  TEST_ASSERT_FALSE(service.interestedIn(e));
}

void test_not_interested_in_logic_tempo_save() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(service.interestedIn(e));
}

void test_not_interested_in_memory_events() {
  LogicalState logicalState;
  MockedClock mockClock;
  MockLed mockLed;
  MenuService service(logicalState, mockLed, mockClock);

  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kMenu;
  TEST_ASSERT_FALSE(service.interestedIn(e));
}

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_publishes_updated_event);
  RUN_TEST(test_init_starts_unlocked_program_mode);
  RUN_TEST(test_init_starts_locked_preset_mode);
  RUN_TEST(test_init_syncs_save_bank_preset_to_logical_state);
  RUN_TEST(test_init_initializes_led);
  RUN_TEST(test_init_active_sets_led_on);
  RUN_TEST(test_init_bypass_sets_led_off);

  // Bypass Toggled
  RUN_TEST(test_logic_bypass_toggled_bypass_sets_led_off);
  RUN_TEST(test_logic_bypass_toggled_active_sets_led_on);

  // Lock/Unlock
  RUN_TEST(test_menu_lock_long_press_locks_menu);
  RUN_TEST(test_menu_lock_long_press_unlocks_menu);
  RUN_TEST(test_bypass_toggled_locks_menu);
  RUN_TEST(test_bypass_toggled_ignored_when_locked);
  RUN_TEST(test_bypass_toggled_restored_when_unlocked);

  // Navigation (Selecting Mode)
  RUN_TEST(test_encoder_delta_publishes_updated_when_unlocked);
  RUN_TEST(test_encoder_press_publishes_updated_when_unlocked);
  RUN_TEST(test_encoder_ignored_when_locked);

  // Pot Overlay
  RUN_TEST(test_pot_value_changed_publishes_updated_when_unlocked);
  RUN_TEST(test_pot_value_changed_publishes_updated_when_locked);

  // Tempo Overlay
  RUN_TEST(test_tempo_change_publishes_updated_when_unlocked);
  RUN_TEST(test_tempo_change_publishes_updated_when_locked);

  // Preset Save Overlay
  RUN_TEST(test_menu_encoder_long_press_publishes_updated_when_unlocked);
  RUN_TEST(test_ui_preset_setting_changed_sets_logical_state_target_bank);
  RUN_TEST(test_ui_preset_setting_changed_sets_logical_state_target_preset);

  // Editing Mode
  RUN_TEST(test_editing_encoder_delta_publishes_updated);
  RUN_TEST(test_editing_encoder_press_exits_editing);

  // Timeout
  RUN_TEST(test_update_not_locks_menu_after_timeout_program_mode);
  RUN_TEST(test_update_locks_menu_after_timeout_preset_mode);
  RUN_TEST(test_update_does_not_lock_before_timeout_preset_mode);
  RUN_TEST(test_update_pops_pot_overlay_after_timeout);
  RUN_TEST(test_update_pops_tempo_overlay_after_timeout);
  RUN_TEST(test_update_does_nothing_when_locked);

  // Program change
  RUN_TEST(test_program_change_publishes_updated);
  RUN_TEST(test_program_change_syncs_save_bank_preset_to_logical_state_preset_mode);
  RUN_TEST(test_program_change_not_syncs_save_bank_preset_to_logical_state_program_mode);

  // interestedIn
  RUN_TEST(test_interested_in_physical_switch_long_press_menu_lock);
  RUN_TEST(test_interested_in_physical_switch_press_menu_encoder);
  RUN_TEST(test_interested_in_physical_encoder_move_menu_encoder);
  RUN_TEST(test_interested_in_logic_pot_value_changed);
  RUN_TEST(test_interested_in_logic_tempo_value_changed);
  RUN_TEST(test_interested_in_logic_bypass_toggled);
  RUN_TEST(test_interested_in_logic_program_changed);
  RUN_TEST(test_interested_in_ui_preset_seting_changed);
  RUN_TEST(test_not_interested_in_other_switches);
  RUN_TEST(test_not_interested_in_switch_press);
  RUN_TEST(test_not_interested_in_other_encoders);
  RUN_TEST(test_not_interested_in_events_it_publishes);
  RUN_TEST(test_not_interested_in_logic_tempo_save);
  RUN_TEST(test_not_interested_in_memory_events);

  UNITY_END();
}
