#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/preset_service.h"

#include "../src/logic/preset_handler.cpp"
#include "../src/services/preset_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_preset_change() {
  LogicalState logicalState;
  PresetService presetService(logicalState);

  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);

  presetService.handleEvent({EventType::kMenuPresetChanged, 0, {.delta=1}});

  TEST_ASSERT_EQUAL(1, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);

  presetService.handleEvent({EventType::kMenuPresetChanged, 0, {.delta=-1}});

  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);
}

void test_preset_bank_change() {
  LogicalState logicalState;
  PresetService presetService(logicalState);

  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);

  presetService.handleEvent({EventType::kMenuPresetChanged, 0, {.delta=1}});

  TEST_ASSERT_EQUAL(1, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);

  presetService.handleEvent({EventType::kMenuPresetBankChanged, 0, {.delta=1}});

  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(1, logicalState.m_currentPresetBank);

  presetService.handleEvent({EventType::kMenuPresetBankChanged, 0, {.delta=-1}});

  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);
}

void test_wrap_around() {
  LogicalState logicalState;
  PresetService presetService(logicalState);

  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);

  presetService.handleEvent({EventType::kMenuPresetChanged, 0, {.delta=-1}});

  TEST_ASSERT_EQUAL(3, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);

  presetService.handleEvent({EventType::kMenuPresetChanged, 0, {.delta=1}});

  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);

  presetService.handleEvent({EventType::kMenuPresetBankChanged, 0, {.delta=-1}});

  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(15, logicalState.m_currentPresetBank);

  presetService.handleEvent({EventType::kMenuPresetBankChanged, 0, {.delta=1}});

  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_preset_change);
  RUN_TEST(test_preset_bank_change);
  RUN_TEST(test_wrap_around);
  UNITY_END();
}