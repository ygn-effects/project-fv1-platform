#include <unity.h>
#include "core/event_bus.h"
#include "drivers/eeprom_m95.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "logic/preset_handler.h"
#include "services/memory_service.h"

#include "../src/drivers/eeprom_m95.cpp"
#include "../src/logic/memory_handler.cpp"
#include "../src/logic/preset_handler.cpp"
#include "../src/services/memory_service.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}
void test_logical_state() {
  LogicalState logicalState;
  hal::DigitalGpioDriver eepromCsPin(0, hal::GpioConfig::kOutput);
  hal::M95Driver eeprom(eepromCsPin);

  MemoryService memoryService(logicalState, eeprom);
  memoryService.init();

  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_currentProgram = 2;
  memoryService.handleEvent({EventType::kSaveLogicalState, 0, {}});

  logicalState.m_bypassState = BypassState::kBypassed;
  logicalState.m_currentProgram = 0;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
}

void setup() {
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_logical_state);
  UNITY_END();
}

void loop() {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(500);
}
