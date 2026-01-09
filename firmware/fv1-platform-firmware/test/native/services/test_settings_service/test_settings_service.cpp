#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "services/settings_service.h"
#include "mock/mock_eeprom.h"
#include <algorithm>

#include "../src/logic/memory_handler.cpp"
#include "../src/services/settings_service.cpp"

// =============================================================================
// Helper functions
// =============================================================================

Event makeMemorySaveEvent(EventSubject t_subject, PotId t_id = PotId::kPot0) {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = t_subject;
  e.m_action = EventAction::kSave;
  e.m_id = static_cast<uint8_t>(t_id);
  return e;
}

Event makeMemoryLoadGeneralEvent() {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kLoad;
  return e;
}

void assertEventBusEmpty() {
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

void setUp() {
  clearEventBus();
}

void tearDown() {

}

// =============================================================================
// Init tests
// =============================================================================

void test_init_loads_logical_state_from_eeprom() {
  MockEEPROM eeprom;
  eeprom.reset();

  // Create first service and save some state to EEPROM
  {
    LogicalState logicalState;
    logicalState.m_bypassState = BypassState::kBypassed;
    logicalState.m_currentProgram = 3;
    logicalState.m_programMode = ProgramMode::kPreset;
    logicalState.m_tempo = 750;

    SettingsService settingsService(logicalState, eeprom);
    settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kGeneral));
  }

  // Create new service with different state
  LogicalState newLogicalState;
  newLogicalState.m_bypassState = BypassState::kActive;
  newLogicalState.m_currentProgram = 0;
  newLogicalState.m_programMode = ProgramMode::kProgram;
  newLogicalState.m_tempo = 0;

  SettingsService newSettingsService(newLogicalState, eeprom);

  // Init should load from EEPROM
  newSettingsService.init();

  // Verify state was restored
  TEST_ASSERT_EQUAL(BypassState::kBypassed, newLogicalState.m_bypassState);
  TEST_ASSERT_EQUAL(3, newLogicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, newLogicalState.m_programMode);
  TEST_ASSERT_EQUAL(750, newLogicalState.m_tempo);
}

// =============================================================================
// Save tests
// =============================================================================

void test_memory_save_bypass() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  // Set bypass state and send the save event
  logicalState.m_bypassState = BypassState::kBypassed;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kBypass));

  // Modify the bypass state
  logicalState.m_bypassState = BypassState::kActive;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
}

void test_memory_save_program_mode() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  // Set program modeand send the save event
  logicalState.m_programMode = ProgramMode::kPreset;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kProgramMode));

  // Modify the program mode
  logicalState.m_programMode = ProgramMode::kProgram;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
}

void test_memory_save_program() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  // Set program and send the save event
  logicalState.m_currentProgram = 1;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kProgram));

  // Modify the program
  logicalState.m_currentProgram = 0;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);
}

void test_memory_save_tap() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  // Set tap parameters and send the save event
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 400;
  logicalState.m_divInterval = 200;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTap));

  // Modify the tap parameters
  logicalState.m_tapState = TapState::kDisabled;
  logicalState.m_divState = DivState::kDisabled;
  logicalState.m_divValue = DivValue::kQuarter;
  logicalState.m_interval = 0;
  logicalState.m_divInterval = 0;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(400, logicalState.m_interval);
  TEST_ASSERT_EQUAL(200, logicalState.m_divInterval);
}

void test_memory_save_tempo() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  // Set tempo and send the save event
  logicalState.m_tempo = 500;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTempo));

  // Modify the tempo
  logicalState.m_tempo = 0;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(500, logicalState.m_tempo);
}

void test_memory_save_expr() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  // Set expr parameters and send the save event
  logicalState.m_currentProgram = 2;
  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot1;
  logicalState.m_exprParams[2].m_direction = Direction::kInverted;
  logicalState.m_exprParams[2].m_heelValue = 256;
  logicalState.m_exprParams[2].m_toeValue = 512;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kExpr));

  // Modify the expr parameters
  logicalState.m_currentProgram = 0;
  logicalState.m_exprParams[2].m_state = ExprState::kInactive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[2].m_direction = Direction::kNormal;
  logicalState.m_exprParams[2].m_heelValue = 0;
  logicalState.m_exprParams[2].m_toeValue = 1023;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[2].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kPot1, logicalState.m_exprParams[2].m_mappedPot);
  TEST_ASSERT_EQUAL(Direction::kInverted, logicalState.m_exprParams[2].m_direction);
  TEST_ASSERT_EQUAL(256, logicalState.m_exprParams[2].m_heelValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_exprParams[2].m_toeValue);
}

void test_memory_save_pot() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  // Set pot parameters and send the save event
  logicalState.m_currentProgram = 3;
  logicalState.m_potParams[3][2].m_state = PotState::kDisabled;
  logicalState.m_potParams[3][2].m_value = 512;
  logicalState.m_potParams[3][2].m_minValue = 256;
  logicalState.m_potParams[3][2].m_maxValue = 768;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, PotId::kPot2));

  // Modify the pot parameters
  logicalState.m_currentProgram = 0;
  logicalState.m_potParams[3][2].m_state = PotState::kActive;
  logicalState.m_potParams[3][2].m_value = 0;
  logicalState.m_potParams[3][2].m_minValue = 0;
  logicalState.m_potParams[3][2].m_maxValue = 0;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[3][2].m_state);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[3][2].m_value);
  TEST_ASSERT_EQUAL(256, logicalState.m_potParams[3][2].m_minValue);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[3][2].m_maxValue);
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_memory() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kMemory;

  TEST_ASSERT_TRUE(settingsService.interestedIn(e));
}

void test_interested_in_memory_but_not_preset() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPreset;

  TEST_ASSERT_FALSE(settingsService.interestedIn(e));
}

void test_interested_in_memory_but_not_preset_bank() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPresetBank;

  TEST_ASSERT_FALSE(settingsService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  SettingsService settingsService(logicalState, eeprom);

  Event e;

  // Not interested in physical switch events
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  TEST_ASSERT_FALSE(settingsService.interestedIn(e));

  // Not interested in logic expr events (outputs them, doesn't consume)
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPot;
  TEST_ASSERT_FALSE(settingsService.interestedIn(e));

  // Not interested in other MIDI events
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(settingsService.interestedIn(e));

  // Not interested in logic Expr events
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(settingsService.interestedIn(e));

  // Not interested in logic program with different action
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(settingsService.interestedIn(e));

  // Not interested in nonsensical event
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kPressed;
  TEST_ASSERT_FALSE(settingsService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();

  // Init tests
  RUN_TEST(test_init_loads_logical_state_from_eeprom);

  // Save tests
  RUN_TEST(test_memory_save_bypass);
  RUN_TEST(test_memory_save_program_mode);
  RUN_TEST(test_memory_save_program);
  RUN_TEST(test_memory_save_tap);
  RUN_TEST(test_memory_save_tempo);
  RUN_TEST(test_memory_save_expr);
  RUN_TEST(test_memory_save_pot);

  //interestedIn Tests
  RUN_TEST(test_interested_in_memory);
  RUN_TEST(test_interested_in_memory_but_not_preset);
  RUN_TEST(test_interested_in_memory_but_not_preset_bank);
  RUN_TEST(test_not_interested_in_other_events);

  UNITY_END();
}
