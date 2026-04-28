#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "mock/mock_clock.h"
#include "mock/mock_serial.h"
#include "services/midi_service.h"

#include "../src/services/midi_service.cpp"
#include "../src/logic/midi_handler.cpp"

// =============================================================================
// Helper Functions
// =============================================================================

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

// MIDI byte helpers - makes raw MIDI protocol more readable
namespace MidiBytes {
  constexpr uint8_t c_ccStatus = 0xB0;
  constexpr uint8_t c_pcStatus = 0xC0;

  // CC numbers matching c_ccMap in midi_service.h
  constexpr uint8_t c_ccPot0 = 0x00;
  constexpr uint8_t c_ccPot1 = 0x01;
  constexpr uint8_t c_ccPot2 = 0x02;
  constexpr uint8_t c_ccMixPot = 0x03;
  constexpr uint8_t c_ccBypass = 0x04;
  constexpr uint8_t c_ccTap = 0x05;
  constexpr uint8_t c_ccTempo = 0x06;
  constexpr uint8_t c_ccProgramMode = 0x07;
}

void pushCCMessage(MockedSerial& t_serial, uint8_t t_cc, uint8_t t_value) {
  t_serial.feedByte(MidiBytes::c_ccStatus);
  t_serial.feedByte(t_cc);
  t_serial.feedByte(t_value);
}

void pushPCMessage(MockedSerial& t_serial, uint8_t t_program) {
  t_serial.feedByte(MidiBytes::c_pcStatus);
  t_serial.feedByte(t_program);
}

Event makeLogicProgramChangedEvent(uint8_t t_programId) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_id = t_programId;
  return e;
}

Event makeUIMidiChannelSettingChanged(int16_t t_delta) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMidiChannel;
  e.m_action = EventAction::kSettingChanged;
  e.m_data.delta = t_delta;
  return e;
}

// Assertion helpers
void assertMidiPotEventPublished(PotId t_potId, uint16_t t_value) {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected MIDI pot event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(t_potId));
  TEST_ASSERT_EQUAL(t_value, e.m_data.value);
}

void assertMidiSwitchEventPublished(SwitchId t_switchId, uint16_t t_value) {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected MIDI switch event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_TRUE(e.matchesId(t_switchId));
  TEST_ASSERT_EQUAL(t_value, e.m_data.value);
}

void assertMidiProgramEventPublished(uint8_t t_program) {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected MIDI program event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_EQUAL(t_program, e.m_data.value);
}

void assertMidiTempoEventPublished(uint16_t t_value) {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected MIDI tempo event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kTempo, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_EQUAL(t_value, e.m_data.value);
}

void assertMidiPresetEventPublished(uint16_t t_value) {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected MIDI preset event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPreset, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_EQUAL(t_value, e.m_data.value);
}

void assertMidiProgramModeEventPublished(uint16_t t_value) {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected MIDI program mode event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMidi, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgramMode, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_EQUAL(t_value, e.m_data.value);
}

void assertMemorySaveEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected memory save event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kGeneral, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
}

void assertEventBusEmpty() {
  TEST_ASSERT_FALSE_MESSAGE(EventBus::hasEvent(), "Expected empty event bus but found event");
}

void setUp() {
  clearEventBus();
}

void tearDown() {
}

// =============================================================================
// Init Tests
// =============================================================================

void test_init_syncs_midi_channel_to_handler() {
  LogicalState logicalState;
  MockedClock clock;

  logicalState.m_midiChannel = 5;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();

  // Send a CC on channel 5 - should be processed
  serial.feedByte(0xB5);  // CC on channel 5
  serial.feedByte(MidiBytes::c_ccPot0);
  serial.feedByte(64);

  midiService.update();

  assertMidiPotEventPublished(PotId::kPot0, 64);
  assertEventBusEmpty();
}

// =============================================================================
// CC Message Tests - Pot Controls
// =============================================================================

void test_cc_pot0_publishes_midi_pot_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccPot0, 64);
  midiService.update();

  assertMidiPotEventPublished(PotId::kPot0, 64);
  assertEventBusEmpty();
}

void test_cc_pot1_publishes_midi_pot_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccPot1, 127);
  midiService.update();

  assertMidiPotEventPublished(PotId::kPot1, 127);
  assertEventBusEmpty();
}

void test_cc_pot2_publishes_midi_pot_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccPot2, 32);
  midiService.update();

  assertMidiPotEventPublished(PotId::kPot2, 32);
  assertEventBusEmpty();
}

void test_cc_mix_pot_publishes_midi_pot_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccMixPot, 100);
  midiService.update();

  assertMidiPotEventPublished(PotId::kMixPot, 100);
  assertEventBusEmpty();
}

// =============================================================================
// CC Message Tests - Switch Controls
// =============================================================================

void test_cc_bypass_enable_publishes_switch_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccBypass, MidiCCValues::c_bypassEnable);
  midiService.update();

  assertMidiSwitchEventPublished(SwitchId::kBypass, MidiCCValues::c_bypassEnable);
  assertEventBusEmpty();
}

void test_cc_bypass_disable_publishes_switch_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccBypass, MidiCCValues::c_bypassDisable);
  midiService.update();

  assertMidiSwitchEventPublished(SwitchId::kBypass, MidiCCValues::c_bypassDisable);
  assertEventBusEmpty();
}

void test_cc_tap_short_press_publishes_switch_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccTap, MidiCCValues::c_tapShortPress);
  midiService.update();

  assertMidiSwitchEventPublished(SwitchId::kTap, MidiCCValues::c_tapShortPress);
  assertEventBusEmpty();
}

void test_cc_tap_long_press_publishes_switch_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccTap, MidiCCValues::c_tapLongPress);
  midiService.update();

  assertMidiSwitchEventPublished(SwitchId::kTap, MidiCCValues::c_tapLongPress);
  assertEventBusEmpty();
}

// =============================================================================
// CC Message Tests - Other Controls
// =============================================================================

void test_cc_tempo_publishes_tempo_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccTempo, 120);
  midiService.update();

  assertMidiTempoEventPublished(120);
  assertEventBusEmpty();
}

void test_cc_program_mode_publishes_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushCCMessage(serial, MidiBytes::c_ccProgramMode, MidiCCValues::c_presetMode);
  midiService.update();

  assertMidiProgramModeEventPublished(MidiCCValues::c_presetMode);
  assertEventBusEmpty();
}

// =============================================================================
// PC Message Tests
// =============================================================================

void test_pc_publishes_program_event_in_program_mode() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushPCMessage(serial, 3);
  midiService.update();

  assertMidiProgramEventPublished(3);
  assertEventBusEmpty();
}

void test_pc_publishes_preset_event_in_preset_mode() {
  LogicalState logicalState;
  logicalState.m_programMode = ProgramMode::kPreset;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();
  pushPCMessage(serial, 3);
  midiService.update();

  assertMidiPresetEventPublished(3);
  assertEventBusEmpty();
}


void test_pc_with_various_program_values() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();

  // Test program 0
  pushPCMessage(serial, 0);
  midiService.update();
  assertMidiProgramEventPublished(0);
  assertEventBusEmpty();

  // Test program 7 (max for FV1)
  pushPCMessage(serial, 7);
  midiService.update();
  assertMidiProgramEventPublished(7);
  assertEventBusEmpty();

  // Test program 127 (max MIDI value)
  pushPCMessage(serial, 127);
  midiService.update();
  assertMidiProgramEventPublished(127);
  assertEventBusEmpty();
}

// =============================================================================
// Incomplete/Invalid Message Tests
// =============================================================================

void test_cc_incomplete_message_no_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();

  // Only status and CC number, missing value
  serial.feedByte(MidiBytes::c_ccStatus);
  serial.feedByte(MidiBytes::c_ccPot0);

  midiService.update();

  assertEventBusEmpty();
}

void test_cc_only_status_byte_no_event() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();

  // Only status byte
  serial.feedByte(MidiBytes::c_ccStatus);

  midiService.update();

  assertEventBusEmpty();
}

void test_cc_recovery_after_incomplete_message() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();

  // Incomplete message
  serial.feedByte(MidiBytes::c_ccStatus);
  serial.feedByte(MidiBytes::c_ccPot0);
  midiService.update();
  assertEventBusEmpty();

  // Complete message should work
  pushCCMessage(serial, MidiBytes::c_ccPot2, 32);
  midiService.update();

  assertMidiPotEventPublished(PotId::kPot2, 32);
  assertEventBusEmpty();
}

void test_cc_out_of_range_ignored() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();

  // CC 10 is outside the c_ccMap (0-9)
  pushCCMessage(serial, 10, 64);
  midiService.update();

  assertEventBusEmpty();
}

void test_message_wrong_channel_ignored() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();

  // CC messages on invalid channel
  serial.feedByte(0xB1); // CC Channel 1
  serial.feedByte(MidiBytes::c_ccBypass);
  serial.feedByte(MidiCCValues::c_bypassDisable);
  midiService.update();

  assertEventBusEmpty();
}

// =============================================================================
// Successive Message Tests
// =============================================================================

void test_successive_cc_messages_processed() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  midiService.init();

  // Queue two messages
  pushCCMessage(serial, MidiBytes::c_ccPot0, 64);
  pushCCMessage(serial, MidiBytes::c_ccPot1, 127);

  // First update processes first message
  midiService.update();
  assertMidiPotEventPublished(PotId::kPot0, 64);
  assertEventBusEmpty();

  // Second update processes second message
  midiService.update();
  assertMidiPotEventPublished(PotId::kPot1, 127);
  assertEventBusEmpty();
}

// =============================================================================
// handleEvent Tests - MIDI Channel Change
// =============================================================================

void test_midi_channel_change_updates_state() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  logicalState.m_midiChannel = 0;
  midiService.init();

  midiService.handleEvent(makeUIMidiChannelSettingChanged(1));

  TEST_ASSERT_EQUAL(1, logicalState.m_midiChannel);
  assertMemorySaveEventPublished();
  assertEventBusEmpty();
}

void test_midi_channel_change_wraps_at_max() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  // Valid channels are 0 to (max-1), so start at max-1
  logicalState.m_midiChannel = MidiHandlerConstants::c_maxMidiChannels - 1;
  midiService.init();

  midiService.handleEvent(makeUIMidiChannelSettingChanged(1));

  TEST_ASSERT_EQUAL(0, logicalState.m_midiChannel);
  assertMemorySaveEventPublished();
  assertEventBusEmpty();
}

void test_midi_channel_change_wraps_at_min() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  logicalState.m_midiChannel = 0;
  midiService.init();

  midiService.handleEvent(makeUIMidiChannelSettingChanged(-1));

  // Wraps to max-1 since valid channels are 0 to (max-1)
  TEST_ASSERT_EQUAL(MidiHandlerConstants::c_maxMidiChannels - 1, logicalState.m_midiChannel);
  assertMemorySaveEventPublished();
  assertEventBusEmpty();
}

void test_midi_channel_change_syncs_handler() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  logicalState.m_midiChannel = 0;
  midiService.init();

  // Change to channel 5
  midiService.handleEvent(makeUIMidiChannelSettingChanged(5));
  clearEventBus();

  // Messages on channel 5 should now be processed
  serial.feedByte(0xB5);  // CC on channel 5
  serial.feedByte(MidiBytes::c_ccPot0);
  serial.feedByte(100);

  midiService.update();

  assertMidiPotEventPublished(PotId::kPot0, 100);
  assertEventBusEmpty();
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_ui_midi_channel_change() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMidiChannel;
  e.m_action = EventAction::kSettingChanged;

  TEST_ASSERT_TRUE(midiService.interestedIn(e));
}

void test_not_interested_in_logic_program_change() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(midiService.interestedIn(e));
}

void test_not_interested_in_midi_pot_events() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  // Service publishes these, should not listen to them
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(midiService.interestedIn(e));
}

void test_not_interested_in_midi_switch_events() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  // Service publishes these, should not listen to them
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(midiService.interestedIn(e));
}

void test_not_interested_in_midi_program_events() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  // Service publishes these via PC, should not listen to them
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(midiService.interestedIn(e));
}

void test_not_interested_in_physical_events() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(midiService.interestedIn(e));
}

void test_not_interested_in_logic_program_save() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  // Only interested in kValueChanged, not kSave
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;

  TEST_ASSERT_FALSE(midiService.interestedIn(e));
}

void test_not_interested_in_memory_events() {
  LogicalState logicalState;
  MockedClock clock;
  MockedSerial serial;
  MidiService midiService(logicalState, serial, clock);

  // Service publishes these, should not listen to them
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kSave;

  TEST_ASSERT_FALSE(midiService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_syncs_midi_channel_to_handler);

  // CC Message - Pot Controls
  RUN_TEST(test_cc_pot0_publishes_midi_pot_event);
  RUN_TEST(test_cc_pot1_publishes_midi_pot_event);
  RUN_TEST(test_cc_pot2_publishes_midi_pot_event);
  RUN_TEST(test_cc_mix_pot_publishes_midi_pot_event);

  // CC Message - Switch Controls
  RUN_TEST(test_cc_bypass_enable_publishes_switch_event);
  RUN_TEST(test_cc_bypass_disable_publishes_switch_event);
  RUN_TEST(test_cc_tap_short_press_publishes_switch_event);
  RUN_TEST(test_cc_tap_long_press_publishes_switch_event);

  // CC Message - Other Controls
  RUN_TEST(test_cc_tempo_publishes_tempo_event);
  RUN_TEST(test_cc_program_mode_publishes_event);

  // PC Message
  RUN_TEST(test_pc_publishes_program_event_in_program_mode);
  RUN_TEST(test_pc_publishes_preset_event_in_preset_mode);
  RUN_TEST(test_pc_with_various_program_values);

  // Incomplete/Invalid Messages
  RUN_TEST(test_cc_incomplete_message_no_event);
  RUN_TEST(test_cc_only_status_byte_no_event);
  RUN_TEST(test_cc_recovery_after_incomplete_message);
  RUN_TEST(test_cc_out_of_range_ignored);
  RUN_TEST(test_message_wrong_channel_ignored);

  // Successive Messages
  RUN_TEST(test_successive_cc_messages_processed);

  // handleEvent - MIDI Channel Change
  RUN_TEST(test_midi_channel_change_updates_state);
  RUN_TEST(test_midi_channel_change_wraps_at_max);
  RUN_TEST(test_midi_channel_change_wraps_at_min);
  RUN_TEST(test_midi_channel_change_syncs_handler);

  // interestedIn
  RUN_TEST(test_interested_in_ui_midi_channel_change);
  RUN_TEST(test_not_interested_in_logic_program_change);
  RUN_TEST(test_not_interested_in_midi_pot_events);
  RUN_TEST(test_not_interested_in_midi_switch_events);
  RUN_TEST(test_not_interested_in_midi_program_events);
  RUN_TEST(test_not_interested_in_physical_events);
  RUN_TEST(test_not_interested_in_logic_program_save);
  RUN_TEST(test_not_interested_in_memory_events);

  UNITY_END();
}
