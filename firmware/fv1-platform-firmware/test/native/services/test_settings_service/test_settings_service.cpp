#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "services/settings_service.h"
#include "mock/mock_clock.h"
#include "mock/mock_eeprom.h"
#include <algorithm>

#include "../src/logic/memory_handler.cpp"
#include "../src/services/settings_service.cpp"

// =============================================================================
// Helper functions
// =============================================================================

Event makeMemorySaveEvent(EventSubject t_subject, uint32_t t_timestamp, PotId t_id = PotId::kPot0) {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = t_subject;
  e.m_action = EventAction::kSave;
  e.m_timestamp = t_timestamp;
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

    MockedClock clock;
    SettingsService settingsService(logicalState, eeprom, clock);
    settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kGeneral, 0));
  }

  // Create new service with different state
  LogicalState newLogicalState;
  newLogicalState.m_bypassState = BypassState::kActive;
  newLogicalState.m_currentProgram = 0;
  newLogicalState.m_programMode = ProgramMode::kProgram;
  newLogicalState.m_tempo = 0;

  MockedClock clock;
  SettingsService newSettingsService(newLogicalState, eeprom, clock);

  // Init should load from EEPROM
  newSettingsService.init();

  // Verify state was restored
  TEST_ASSERT_EQUAL(BypassState::kBypassed, newLogicalState.m_bypassState);
  TEST_ASSERT_EQUAL(3, newLogicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, newLogicalState.m_programMode);
  TEST_ASSERT_EQUAL(750, newLogicalState.m_tempo);
}

// =============================================================================
// Edit timeout tests
// =============================================================================

void test_system_state_request_committed_immediately() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set state
  logicalState.m_bypassState = BypassState::kBypassed;
  logicalState.m_currentProgram = 1;
  logicalState.m_tempo = 500;
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_interval = 500;

  // Send the save event
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kGeneral, 0));
  settingsService.update();

  // Reset values
  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_currentProgram = 0;
  logicalState.m_tempo = 0;
  logicalState.m_tapState = TapState::kDisabled;
  logicalState.m_interval = 0;

  // Reload from persistence
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logical state
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(500, logicalState.m_tempo);
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(500, logicalState.m_interval);
}

void test_request_not_committed_if_not_edit_timeout() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Create first service and save some state to EEPROM
  {
    LogicalState logicalState;
    logicalState.m_bypassState = BypassState::kActive;

    MockedClock clock;
    SettingsService settingsService(logicalState, eeprom, clock);
    settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kGeneral, 0));
  }

  // Set bypass state and send the save event
  logicalState.m_bypassState = BypassState::kBypassed;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kBypass, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout - 1);
  settingsService.update();

  // Modify the bypass state
  logicalState.m_bypassState = BypassState::kActive;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
}

void test_request_committed_if_edit_timeout() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set bypass state and send the save event
  logicalState.m_bypassState = BypassState::kBypassed;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kBypass, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Modify the bypass state
  logicalState.m_bypassState = BypassState::kActive;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
}

void test_successive_requests_not_commited_if_not_edit_timeout() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Create first service and save some state to EEPROM
  {
    LogicalState logicalState;
    logicalState.m_bypassState = BypassState::kActive;
    logicalState.m_currentProgram = 0;
    logicalState.m_tempo = 0;

    MockedClock clock;
    SettingsService settingsService(logicalState, eeprom, clock);
    settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kGeneral, 0));
  }

  // Set bypass state and send the save event
  logicalState.m_bypassState = BypassState::kBypassed;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kBypass, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(0);
  settingsService.update();

  // Set current program and send the save event
  logicalState.m_currentProgram = 2;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kProgram, 500));

  // Set the clock and update to trigger the commit
  clock.advanceBy(500);
  settingsService.update();

  // Set tempo and send the save event
  logicalState.m_tempo = 200;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTempo, 1000));

  // Set the clock and update to trigger the commit
  clock.advanceBy(500);
  settingsService.update();

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout - 1);
  settingsService.update();

  // Reset the values and restore
  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_currentProgram = 0;
  logicalState.m_tempo = 0;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Test logical state
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(0, logicalState.m_tempo);
}

void test_successive_requests_committed_if_edit_timeout() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set bypass state and send the save event
  logicalState.m_bypassState = BypassState::kBypassed;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kBypass, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(0);
  settingsService.update();

  // Set current program and send the save event
  logicalState.m_currentProgram = 2;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kProgram, 500));

  // Set the clock and update to trigger the commit
  clock.advanceBy(500);
  settingsService.update();

  // Set tempo and send the save event
  logicalState.m_tempo = 200;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTempo, 1000));

  // Set the clock and update to trigger the commit
  clock.advanceBy(1000);
  settingsService.update();

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Reset the values and restore
  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_currentProgram = 0;
  logicalState.m_tempo = 0;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Test logical state
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(2, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(200, logicalState.m_tempo);
}

void test_only_requests_committed_if_edit_timeout() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Create first service and save some state to EEPROM
  {
    LogicalState logicalState;
    logicalState.m_bypassState = BypassState::kActive;
    logicalState.m_currentProgram = 1;
    logicalState.m_tempo = 100;
    logicalState.m_tapState = TapState::kEnabled;
    logicalState.m_interval = 500;

    MockedClock clock;
    SettingsService settingsService(logicalState, eeprom, clock);
    settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kGeneral, 0));
  }

  // Set state
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_interval = 500;

  // Set bypass state and send the save event
  logicalState.m_bypassState = BypassState::kBypassed;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kBypass, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(0);
  settingsService.update();

  // Set current program and send the save event
  logicalState.m_currentProgram = 2;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kProgram, 500));

  // Set the clock and update to trigger the commit
  clock.advanceBy(500);
  settingsService.update();

  // Set tempo and send the save event
  logicalState.m_tempo = 200;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTempo, 1000));

  // Set the clock and update to trigger the commit
  clock.advanceBy(1000);
  settingsService.update();

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Restore
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Test logical state
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(2, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(200, logicalState.m_tempo);
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(500, logicalState.m_interval);
}

// =============================================================================
// Save after edit timeout tests
// =============================================================================

void test_memory_save_bypass() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set bypass state and send the save event
  logicalState.m_bypassState = BypassState::kBypassed;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kBypass, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

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
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set program modeand send the save event
  logicalState.m_programMode = ProgramMode::kPreset;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kProgramMode, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

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
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set program and send the save event
  logicalState.m_currentProgram = 1;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kProgram, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

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
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set tap parameters and send the save event
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 400;
  logicalState.m_divInterval = 200;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTap, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

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
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set tempo and send the save event
  logicalState.m_tempo = 500;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTempo, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

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
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set expr parameters and send the save event
  logicalState.m_currentProgram = 2;
  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot1;
  logicalState.m_exprParams[2].m_direction = Direction::kInverted;
  logicalState.m_exprParams[2].m_heelValue = 256;
  logicalState.m_exprParams[2].m_toeValue = 512;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kExpr, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

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

void test_memory_save_pot0() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set pot parameters and send the save event
  logicalState.m_currentProgram = 3;
  logicalState.m_potParams[3][0].m_state = PotState::kDisabled;
  logicalState.m_potParams[3][0].m_value = 512;
  logicalState.m_potParams[3][0].m_minValue = 256;
  logicalState.m_potParams[3][0].m_maxValue = 768;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 0, PotId::kPot0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Modify the pot parameters
  logicalState.m_currentProgram = 0;
  logicalState.m_potParams[3][0].m_state = PotState::kActive;
  logicalState.m_potParams[3][0].m_value = 0;
  logicalState.m_potParams[3][0].m_minValue = 0;
  logicalState.m_potParams[3][0].m_maxValue = 0;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[3][0].m_state);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[3][0].m_value);
  TEST_ASSERT_EQUAL(256, logicalState.m_potParams[3][0].m_minValue);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[3][0].m_maxValue);
}

void test_memory_save_pot1() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set pot parameters and send the save event
  logicalState.m_currentProgram = 3;
  logicalState.m_potParams[3][1].m_state = PotState::kDisabled;
  logicalState.m_potParams[3][1].m_value = 512;
  logicalState.m_potParams[3][1].m_minValue = 256;
  logicalState.m_potParams[3][1].m_maxValue = 768;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 0, PotId::kPot1));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Modify the pot parameters
  logicalState.m_currentProgram = 0;
  logicalState.m_potParams[3][1].m_state = PotState::kActive;
  logicalState.m_potParams[3][1].m_value = 0;
  logicalState.m_potParams[3][1].m_minValue = 0;
  logicalState.m_potParams[3][1].m_maxValue = 0;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[3][1].m_state);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[3][1].m_value);
  TEST_ASSERT_EQUAL(256, logicalState.m_potParams[3][1].m_minValue);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[3][1].m_maxValue);
}

void test_memory_save_pot2() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set pot parameters and send the save event
  logicalState.m_currentProgram = 3;
  logicalState.m_potParams[3][2].m_state = PotState::kDisabled;
  logicalState.m_potParams[3][2].m_value = 512;
  logicalState.m_potParams[3][2].m_minValue = 256;
  logicalState.m_potParams[3][2].m_maxValue = 768;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 0, PotId::kPot2));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

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

void test_memory_save_mix_pot() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set pot parameters and send the save event
  logicalState.m_currentProgram = 3;
  logicalState.m_potParams[3][3].m_state = PotState::kDisabled;
  logicalState.m_potParams[3][3].m_value = 512;
  logicalState.m_potParams[3][3].m_minValue = 256;
  logicalState.m_potParams[3][3].m_maxValue = 768;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 0, PotId::kMixPot));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Modify the pot parameters
  logicalState.m_currentProgram = 0;
  logicalState.m_potParams[3][3].m_state = PotState::kActive;
  logicalState.m_potParams[3][3].m_value = 0;
  logicalState.m_potParams[3][3].m_minValue = 0;
  logicalState.m_potParams[3][3].m_maxValue = 0;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[3][3].m_state);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[3][3].m_value);
  TEST_ASSERT_EQUAL(256, logicalState.m_potParams[3][3].m_minValue);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[3][3].m_maxValue);
}

void test_memory_save_multiple_pots() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set pot parameters and send the save event
  logicalState.m_currentProgram = 3;
  logicalState.m_potParams[3][0].m_state = PotState::kDisabled;
  logicalState.m_potParams[3][0].m_value = 512;
  logicalState.m_potParams[3][0].m_minValue = 256;
  logicalState.m_potParams[3][0].m_maxValue = 768;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 0, PotId::kPot0));

  // Set pot parameters and send the save event
  logicalState.m_potParams[3][2].m_state = PotState::kDisabled;
  logicalState.m_potParams[3][2].m_value = 512;
  logicalState.m_potParams[3][2].m_minValue = 256;
  logicalState.m_potParams[3][2].m_maxValue = 768;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 1000, PotId::kPot2));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Modify the pot parameters
  logicalState.m_currentProgram = 0;
  logicalState.m_potParams[3][0].m_state = PotState::kActive;
  logicalState.m_potParams[3][0].m_value = 0;
  logicalState.m_potParams[3][0].m_minValue = 0;
  logicalState.m_potParams[3][0].m_maxValue = 0;
  logicalState.m_potParams[3][2].m_state = PotState::kActive;
  logicalState.m_potParams[3][2].m_value = 0;
  logicalState.m_potParams[3][2].m_minValue = 0;
  logicalState.m_potParams[3][2].m_maxValue = 0;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[3][0].m_state);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[3][0].m_value);
  TEST_ASSERT_EQUAL(256, logicalState.m_potParams[3][0].m_minValue);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[3][0].m_maxValue);
  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[3][2].m_state);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[3][2].m_value);
  TEST_ASSERT_EQUAL(256, logicalState.m_potParams[3][2].m_minValue);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[3][2].m_maxValue);
}

void test_memory_save_multiple_params() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set bypass qnd tempo state and send the save event
  logicalState.m_bypassState = BypassState::kBypassed;
  logicalState.m_tempo = 500;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kBypass, 0));
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTempo, 1000));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Modify the tempo andbypass state
  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_tempo = 0;

  // Send the restore event
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(500, logicalState.m_tempo);
}

// =============================================================================
// Preset Mode: Program parameter saves suppressed
// =============================================================================

void test_preset_mode_ignores_pot_save() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Save pot in program mode
  logicalState.m_currentProgram = 1;
  logicalState.m_potParams[1][0].m_value = 512;
  logicalState.m_potParams[1][0].m_minValue = 100;
  logicalState.m_potParams[1][0].m_maxValue = 900;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 0, PotId::kPot0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Switch to preset mode and try to save different pot values
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_potParams[1][0].m_value = 999;
  logicalState.m_potParams[1][0].m_minValue = 0;
  logicalState.m_potParams[1][0].m_maxValue = 1023;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 0, PotId::kPot0));

  // Set the clock and update to trigger the commit
  clock.setClock(0);
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Load back and verify original values persisted
  logicalState.m_potParams[1][0].m_value = 0;
  logicalState.m_potParams[1][0].m_minValue = 0;
  logicalState.m_potParams[1][0].m_maxValue = 0;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[1][0].m_value);
  TEST_ASSERT_EQUAL(100, logicalState.m_potParams[1][0].m_minValue);
  TEST_ASSERT_EQUAL(900, logicalState.m_potParams[1][0].m_maxValue);
}

void test_preset_mode_ignores_expr_save() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Save expr in program mode
  logicalState.m_currentProgram = 2;
  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot1;
  logicalState.m_exprParams[2].m_heelValue = 256;
  logicalState.m_exprParams[2].m_toeValue = 512;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kExpr, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Switch to preset mode and try to save different expr values
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_exprParams[2].m_state = ExprState::kInactive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[2].m_heelValue = 0;
  logicalState.m_exprParams[2].m_toeValue = 1023;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kExpr, 0));

  // Set the clock and update to trigger the commit
  clock.setClock(0);
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Load back and verify original values persisted
  logicalState.m_exprParams[2].m_state = ExprState::kInactive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[2].m_heelValue = 0;
  logicalState.m_exprParams[2].m_toeValue = 0;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[2].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kPot1, logicalState.m_exprParams[2].m_mappedPot);
  TEST_ASSERT_EQUAL(256, logicalState.m_exprParams[2].m_heelValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_exprParams[2].m_toeValue);
}

void test_preset_mode_ignores_tap_save() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Save tap in program mode
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_interval = 400;
  logicalState.m_divInterval = 200;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTap, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Switch to preset mode and try to save different tap values
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_tapState = TapState::kDisabled;
  logicalState.m_divState = DivState::kDisabled;
  logicalState.m_interval = 800;
  logicalState.m_divInterval = 100;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTap, 0));

  // Set the clock and update to trigger the commit
  clock.setClock(0);
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Load back and verify original values persisted
  logicalState.m_tapState = TapState::kDisabled;
  logicalState.m_divState = DivState::kDisabled;
  logicalState.m_interval = 0;
  logicalState.m_divInterval = 0;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(400, logicalState.m_interval);
  TEST_ASSERT_EQUAL(200, logicalState.m_divInterval);
}

void test_preset_mode_ignores_tempo_save() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Save tempo in program mode
  logicalState.m_tempo = 500;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTempo, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Switch to preset mode and try to save different tempo
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_tempo = 999;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTempo, 0));

  // Set the clock and update to trigger the commit
  clock.setClock(0);
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();


  // Load back and verify original value persisted
  logicalState.m_tempo = 0;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  TEST_ASSERT_EQUAL(500, logicalState.m_tempo);
}

// =============================================================================
// Preset Mode: Global saves still persist
// =============================================================================

void test_preset_mode_still_saves_bypass() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Save bypass in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_bypassState = BypassState::kBypassed;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kBypass, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Modify and reload
  logicalState.m_bypassState = BypassState::kActive;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
}

void test_preset_mode_still_saves_program() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Save program in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_currentProgram = 5;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kProgram, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Modify and reload
  logicalState.m_currentProgram = 0;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);
}

void test_preset_mode_still_saves_program_mode() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Save program mode while in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kProgramMode, 0));

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Modify and reload
  logicalState.m_programMode = ProgramMode::kProgram;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
}

void test_preset_mode_still_saves_general() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  // Set the clock and update to trigger the commit
  clock.advanceBy(SettingsServiceConstants::c_editTimeout);
  settingsService.update();

  // Save general in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_midiChannel = 5;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kGeneral, 0));

  // Modify and reload
  logicalState.m_midiChannel = 0;
  settingsService.handleEvent(makeMemoryLoadGeneralEvent());

  TEST_ASSERT_EQUAL(5, logicalState.m_midiChannel);
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_memory() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  Event e;
  e.m_domain = EventDomain::kMemory;

  TEST_ASSERT_TRUE(settingsService.interestedIn(e));
}

void test_interested_in_memory_but_not_preset() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPreset;

  TEST_ASSERT_FALSE(settingsService.interestedIn(e));
}

void test_interested_in_memory_but_not_preset_bank() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPresetBank;

  TEST_ASSERT_FALSE(settingsService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

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

// =============================================================================
// Preset Dirty Flag tests
// =============================================================================

void test_preset_dirty_flag_starts_false() {
  LogicalState logicalState;
  TEST_ASSERT_FALSE(logicalState.m_presetDirty);
}

void test_preset_mode_pot_save_sets_dirty() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  logicalState.m_programMode = ProgramMode::kPreset;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 0, PotId::kPot0));

  TEST_ASSERT_TRUE(logicalState.m_presetDirty);
}

void test_preset_mode_tap_save_sets_dirty() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  logicalState.m_programMode = ProgramMode::kPreset;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTap, 0));

  TEST_ASSERT_TRUE(logicalState.m_presetDirty);
}

void test_preset_mode_tempo_save_sets_dirty() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  logicalState.m_programMode = ProgramMode::kPreset;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kTempo, 0));

  TEST_ASSERT_TRUE(logicalState.m_presetDirty);
}

void test_preset_mode_expr_save_sets_dirty() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  logicalState.m_programMode = ProgramMode::kPreset;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kExpr, 0));

  TEST_ASSERT_TRUE(logicalState.m_presetDirty);
}

void test_program_mode_pot_save_does_not_set_dirty() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  eeprom.reset();
  MockedClock clock;
  SettingsService settingsService(logicalState, eeprom, clock);

  logicalState.m_programMode = ProgramMode::kProgram;
  settingsService.handleEvent(makeMemorySaveEvent(EventSubject::kPot, 0, PotId::kPot0));

  TEST_ASSERT_FALSE(logicalState.m_presetDirty);
}

int main() {
  UNITY_BEGIN();

  // Init tests
  RUN_TEST(test_init_loads_logical_state_from_eeprom);

  // Edit timeout tests
  RUN_TEST(test_system_state_request_committed_immediately);
  RUN_TEST(test_request_not_committed_if_not_edit_timeout);
  RUN_TEST(test_request_committed_if_edit_timeout);
  RUN_TEST(test_successive_requests_not_commited_if_not_edit_timeout);
  RUN_TEST(test_successive_requests_committed_if_edit_timeout);
  RUN_TEST(test_only_requests_committed_if_edit_timeout);

  // Save after edit timeout tests
  RUN_TEST(test_memory_save_bypass);
  RUN_TEST(test_memory_save_program_mode);
  RUN_TEST(test_memory_save_program);
  RUN_TEST(test_memory_save_tap);
  RUN_TEST(test_memory_save_tempo);
  RUN_TEST(test_memory_save_expr);
  RUN_TEST(test_memory_save_pot0);
  RUN_TEST(test_memory_save_pot1);
  RUN_TEST(test_memory_save_pot2);
  RUN_TEST(test_memory_save_mix_pot);
  RUN_TEST(test_memory_save_multiple_pots);
  RUN_TEST(test_memory_save_multiple_params);

  // Preset Mode: Program parameter saves suppressed
  RUN_TEST(test_preset_mode_ignores_pot_save);
  RUN_TEST(test_preset_mode_ignores_expr_save);
  RUN_TEST(test_preset_mode_ignores_tap_save);
  RUN_TEST(test_preset_mode_ignores_tempo_save);

  // Preset Mode: Global saves still persist
  RUN_TEST(test_preset_mode_still_saves_bypass);
  RUN_TEST(test_preset_mode_still_saves_program);
  RUN_TEST(test_preset_mode_still_saves_program_mode);
  RUN_TEST(test_preset_mode_still_saves_general);

  // Preset Dirty Flag
  RUN_TEST(test_preset_dirty_flag_starts_false);
  RUN_TEST(test_preset_mode_pot_save_sets_dirty);
  RUN_TEST(test_preset_mode_tap_save_sets_dirty);
  RUN_TEST(test_preset_mode_tempo_save_sets_dirty);
  RUN_TEST(test_preset_mode_expr_save_sets_dirty);
  RUN_TEST(test_program_mode_pot_save_does_not_set_dirty);

  //interestedIn Tests
  RUN_TEST(test_interested_in_memory);
  RUN_TEST(test_interested_in_memory_but_not_preset);
  RUN_TEST(test_interested_in_memory_but_not_preset_bank);
  RUN_TEST(test_not_interested_in_other_events);

  UNITY_END();
}
