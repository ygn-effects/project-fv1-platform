#include <unity.h>
#include "logic/midi_handler.h"

#include "../src/logic/midi_handler.cpp"

// =============================================================================
// Helper Functions
// =============================================================================

MidiHandler makeHandlerOnChannel(uint8_t t_channel) {
  MidiHandler h;
  h.setMidiChannel(t_channel);
  return h;
}

void setUp() {}

void tearDown() {}

// =============================================================================
// Initialization Tests
// =============================================================================

void test_default_initialization_no_messages() {
  MidiHandler handler;
  MidiMessage msg;

  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

// =============================================================================
// setMidiChannel Tests
// =============================================================================

void test_setMidiChannel_changes_channel() {
  MidiHandler handler;

  handler.setMidiChannel(5);

  // Send CC on channel 5 (0xB5)
  handler.pushByte(0xB5);  // CC status on channel 5
  handler.pushByte(14);    // CC number
  handler.pushByte(100);   // Value

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(5, msg.m_channel);
}

// =============================================================================
// Control Change (CC) Tests
// =============================================================================

void test_cc_message_complete() {
  MidiHandler handler;

  handler.pushByte(0xB0);  // CC status on channel 0
  handler.pushByte(14);    // CC number
  handler.pushByte(127);   // Value

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(MidiMessageType::kControlChange, msg.m_type);
  TEST_ASSERT_EQUAL(0, msg.m_channel);
  TEST_ASSERT_EQUAL(14, msg.m_param);
  TEST_ASSERT_EQUAL(127, msg.m_value);
}

void test_cc_message_with_zero_value() {
  MidiHandler handler;

  handler.pushByte(0xB0);
  handler.pushByte(20);
  handler.pushByte(0);

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(MidiMessageType::kControlChange, msg.m_type);
  TEST_ASSERT_EQUAL(20, msg.m_param);
  TEST_ASSERT_EQUAL(0, msg.m_value);
}

void test_cc_incomplete_message_no_data() {
  MidiHandler handler;

  handler.pushByte(0xB0);  // Only status, no data

  MidiMessage msg;
  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

void test_cc_incomplete_message_one_data_byte() {
  MidiHandler handler;

  handler.pushByte(0xB0);
  handler.pushByte(14);  // Only one data byte

  MidiMessage msg;
  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

// =============================================================================
// Program Change (PC) Tests
// =============================================================================

void test_pc_message_complete() {
  MidiHandler handler;

  handler.pushByte(0xC0);  // PC status on channel 0
  handler.pushByte(5);     // Program number

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(MidiMessageType::kProgramChange, msg.m_type);
  TEST_ASSERT_EQUAL(0, msg.m_channel);
  TEST_ASSERT_EQUAL(5, msg.m_param);
  TEST_ASSERT_EQUAL(0, msg.m_value);  // PC has no value, should be 0
}

void test_pc_message_max_program() {
  MidiHandler handler;

  handler.pushByte(0xC0);
  handler.pushByte(127);

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(MidiMessageType::kProgramChange, msg.m_type);
  TEST_ASSERT_EQUAL(127, msg.m_param);
}

void test_pc_incomplete_message() {
  MidiHandler handler;

  handler.pushByte(0xC0);  // Only status, no program number

  MidiMessage msg;
  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

// =============================================================================
// Channel Filtering Tests
// =============================================================================

void test_ignores_message_on_wrong_channel() {
  MidiHandler handler;
  handler.setMidiChannel(0);

  handler.pushByte(0xB5);  // CC on channel 5
  handler.pushByte(14);
  handler.pushByte(100);

  MidiMessage msg;
  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

void test_accepts_message_on_correct_channel() {
  MidiHandler handler;
  handler.setMidiChannel(7);

  handler.pushByte(0xB7);  // CC on channel 7
  handler.pushByte(14);
  handler.pushByte(100);

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(7, msg.m_channel);
}

void test_channel_filtering_all_channels() {
  for (uint8_t channel = 0; channel <= 15; channel++) {
    MidiHandler handler;
    handler.setMidiChannel(channel);

    handler.pushByte(0xB0 | channel);  // CC on this channel
    handler.pushByte(1);
    handler.pushByte(64);

    MidiMessage msg;
    TEST_ASSERT_TRUE(handler.popMessage(msg));
    TEST_ASSERT_EQUAL(channel, msg.m_channel);
  }
}

// =============================================================================
// Running Status Tests
// =============================================================================

void test_data_bytes_without_status_ignored() {
  MidiHandler handler;

  // Push data bytes without a status byte first
  handler.pushByte(14);
  handler.pushByte(100);

  MidiMessage msg;
  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

void test_running_status_reset_after_complete_message() {
  MidiHandler handler;

  // Complete CC message
  handler.pushByte(0xB0);
  handler.pushByte(14);
  handler.pushByte(100);

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));

  // Now push data bytes without status - should be ignored
  handler.pushByte(15);
  handler.pushByte(50);

  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

// =============================================================================
// Unsupported Message Types Tests
// =============================================================================

void test_unsupported_message_type_resets_running_status() {
  MidiHandler handler;

  // Note On (0x90) is not supported
  handler.pushByte(0x90);
  handler.pushByte(60);
  handler.pushByte(100);

  MidiMessage msg;
  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

void test_note_off_ignored() {
  MidiHandler handler;

  handler.pushByte(0x80);  // Note Off
  handler.pushByte(60);
  handler.pushByte(0);

  MidiMessage msg;
  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

void test_pitch_bend_ignored() {
  MidiHandler handler;

  handler.pushByte(0xE0);  // Pitch Bend
  handler.pushByte(0);
  handler.pushByte(64);

  MidiMessage msg;
  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

void test_aftertouch_ignored() {
  MidiHandler handler;

  handler.pushByte(0xD0);  // Channel Aftertouch
  handler.pushByte(100);

  MidiMessage msg;
  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

// =============================================================================
// Multiple Messages Tests
// =============================================================================

void test_multiple_cc_messages() {
  MidiHandler handler;

  // First CC
  handler.pushByte(0xB0);
  handler.pushByte(14);
  handler.pushByte(100);

  // Second CC
  handler.pushByte(0xB0);
  handler.pushByte(15);
  handler.pushByte(50);

  MidiMessage msg;

  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(14, msg.m_param);
  TEST_ASSERT_EQUAL(100, msg.m_value);

  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(15, msg.m_param);
  TEST_ASSERT_EQUAL(50, msg.m_value);

  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

void test_mixed_cc_and_pc_messages() {
  MidiHandler handler;

  // CC message
  handler.pushByte(0xB0);
  handler.pushByte(14);
  handler.pushByte(100);

  // PC message
  handler.pushByte(0xC0);
  handler.pushByte(3);

  MidiMessage msg;

  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(MidiMessageType::kControlChange, msg.m_type);
  TEST_ASSERT_EQUAL(14, msg.m_param);

  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(MidiMessageType::kProgramChange, msg.m_type);
  TEST_ASSERT_EQUAL(3, msg.m_param);

  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

// =============================================================================
// Message Buffer Tests
// =============================================================================

void test_message_buffer_fifo_order() {
  MidiHandler handler;

  // Push 3 PC messages
  for (uint8_t i = 0; i < 3; i++) {
    handler.pushByte(0xC0);
    handler.pushByte(i);
  }

  MidiMessage msg;
  for (uint8_t i = 0; i < 3; i++) {
    TEST_ASSERT_TRUE(handler.popMessage(msg));
    TEST_ASSERT_EQUAL(i, msg.m_param);
  }
}

void test_message_buffer_overflow_behavior() {
  MidiHandler handler;

  // Push more messages than buffer can hold (c_maxMidiMessages = 5)
  for (uint8_t i = 0; i < 7; i++) {
    handler.pushByte(0xC0);
    handler.pushByte(i);
  }

  // Should be able to pop up to buffer size
  MidiMessage msg;
  uint8_t count = 0;
  while (handler.popMessage(msg)) {
    count++;
  }

  // Buffer size is 5, so we should get at most 5 messages
  TEST_ASSERT_LESS_OR_EQUAL(MidiHandlerConstants::c_maxMidiMessages, count);
}

// =============================================================================
// Edge Cases Tests
// =============================================================================

void test_status_byte_interrupts_incomplete_message() {
  MidiHandler handler;

  // Start a CC but interrupt with another status
  handler.pushByte(0xB0);
  handler.pushByte(14);
  // Instead of second data byte, send new status
  handler.pushByte(0xC0);
  handler.pushByte(5);

  MidiMessage msg;
  // Only the PC should be complete
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(MidiMessageType::kProgramChange, msg.m_type);
  TEST_ASSERT_EQUAL(5, msg.m_param);

  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

void test_wrong_channel_status_followed_by_correct() {
  MidiHandler handler;
  handler.setMidiChannel(0);

  // Wrong channel
  handler.pushByte(0xB5);
  handler.pushByte(14);
  handler.pushByte(100);

  // Correct channel
  handler.pushByte(0xB0);
  handler.pushByte(15);
  handler.pushByte(50);

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(15, msg.m_param);
  TEST_ASSERT_EQUAL(50, msg.m_value);

  TEST_ASSERT_FALSE(handler.popMessage(msg));
}

void test_boundary_cc_numbers() {
  MidiHandler handler;

  // CC 0 (Bank Select MSB)
  handler.pushByte(0xB0);
  handler.pushByte(0);
  handler.pushByte(64);

  // CC 127
  handler.pushByte(0xB0);
  handler.pushByte(127);
  handler.pushByte(64);

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(0, msg.m_param);

  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(127, msg.m_param);
}

void test_boundary_cc_values() {
  MidiHandler handler;

  // Value 0
  handler.pushByte(0xB0);
  handler.pushByte(14);
  handler.pushByte(0);

  // Value 127
  handler.pushByte(0xB0);
  handler.pushByte(14);
  handler.pushByte(127);

  MidiMessage msg;
  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(0, msg.m_value);

  TEST_ASSERT_TRUE(handler.popMessage(msg));
  TEST_ASSERT_EQUAL(127, msg.m_value);
}

int main() {
  UNITY_BEGIN();

  // Initialization
  RUN_TEST(test_default_initialization_no_messages);

  // setMidiChannel
  RUN_TEST(test_setMidiChannel_changes_channel);

  // Control Change (CC)
  RUN_TEST(test_cc_message_complete);
  RUN_TEST(test_cc_message_with_zero_value);
  RUN_TEST(test_cc_incomplete_message_no_data);
  RUN_TEST(test_cc_incomplete_message_one_data_byte);

  // Program Change (PC)
  RUN_TEST(test_pc_message_complete);
  RUN_TEST(test_pc_message_max_program);
  RUN_TEST(test_pc_incomplete_message);

  // Channel Filtering
  RUN_TEST(test_ignores_message_on_wrong_channel);
  RUN_TEST(test_accepts_message_on_correct_channel);
  RUN_TEST(test_channel_filtering_all_channels);

  // Running Status
  RUN_TEST(test_data_bytes_without_status_ignored);
  RUN_TEST(test_running_status_reset_after_complete_message);

  // Unsupported Message Types
  RUN_TEST(test_unsupported_message_type_resets_running_status);
  RUN_TEST(test_note_off_ignored);
  RUN_TEST(test_pitch_bend_ignored);
  RUN_TEST(test_aftertouch_ignored);

  // Multiple Messages
  RUN_TEST(test_multiple_cc_messages);
  RUN_TEST(test_mixed_cc_and_pc_messages);

  // Message Buffer
  RUN_TEST(test_message_buffer_fifo_order);
  RUN_TEST(test_message_buffer_overflow_behavior);

  // Edge Cases
  RUN_TEST(test_status_byte_interrupts_incomplete_message);
  RUN_TEST(test_wrong_channel_status_followed_by_correct);
  RUN_TEST(test_boundary_cc_numbers);
  RUN_TEST(test_boundary_cc_values);

  UNITY_END();
}
