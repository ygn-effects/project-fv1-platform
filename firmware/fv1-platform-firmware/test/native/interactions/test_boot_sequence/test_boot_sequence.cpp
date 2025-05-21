#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/bypass_service.h"
#include "services/expr_service.h"
#include "services/fsm_service.h"
#include "services/fv1_service.h"
#include "services/memory_service.h"
#include "services/menu_service.h"
#include "services/midi_service.h"
#include "services/pot_service.h"
#include "services/preset_service.h"
#include "services/program_service.h"
#include "services/tap_service.h"
#include "services/tempo_service.h"
#include "mock/mock_eeprom.h"
#include "mock/mock_fv1.h"
#include "mock/mock_bypass.h"

#include "../src/logic/expr_handler.cpp"
#include "../src/logic/fv1_handler.cpp"
#include "../src/logic/memory_handler.cpp"
#include "../src/logic/midi_handler.cpp"
#include "../src/logic/pot_handler.cpp"
#include "../src/logic/preset_handler.cpp"
#include "../src/logic/tap_handler.cpp"
#include "../src/services/bypass_service.cpp"
#include "../src/services/expr_service.cpp"
#include "../src/services/fsm_service.cpp"
#include "../src/services/fv1_service.cpp"
#include "../src/services/memory_service.cpp"
#include "../src/services/menu_service.cpp"
#include "../src/services/midi_service.cpp"
#include "../src/services/pot_service.cpp"
#include "../src/services/preset_service.cpp"
#include "../src/services/program_service.cpp"
#include "../src/services/tap_service.cpp"
#include "../src/services/tempo_service.cpp"
#include "../src/ui/menu_model.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void fillMockEeprom() {
  LogicalState logicalState;
  MockEEPROM mockEeprom;
  MemoryService memoryService(logicalState, mockEeprom);

  mockEeprom.reset();

  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_programMode = ProgramMode::kProgram;
  logicalState.m_currentProgram = 2;
  logicalState.m_currentPreset = 2;
  logicalState.m_currentPresetBank = 5;
  logicalState.m_midiChannel = 7;
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 512;
  logicalState.m_divInterval = 256;
  logicalState.m_tempo = 256;
  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot2;
  logicalState.m_exprParams[2].m_direction = Direction::kNormal;
  logicalState.m_exprParams[2].m_heelValue = 256;
  logicalState.m_exprParams[2].m_toeValue = 512;
  logicalState.m_potParams[2][0].m_state = PotState::kActive;
  logicalState.m_potParams[2][0].m_value = 300;
  logicalState.m_potParams[2][0].m_minValue = 256;
  logicalState.m_potParams[2][0].m_maxValue = 512;
  logicalState.m_potParams[2][1].m_state = PotState::kActive;
  logicalState.m_potParams[2][1].m_value = 300;
  logicalState.m_potParams[2][1].m_minValue = 0;
  logicalState.m_potParams[2][1].m_maxValue = 1023;
  logicalState.m_potParams[2][2].m_state = PotState::kActive;
  logicalState.m_potParams[2][2].m_value = 512;
  logicalState.m_potParams[2][2].m_minValue = 0;
  logicalState.m_potParams[2][2].m_maxValue = 1023;
  logicalState.m_potParams[2][3].m_state = PotState::kActive;
  logicalState.m_potParams[2][3].m_value = 256;
  logicalState.m_potParams[2][3].m_minValue = 0;
  logicalState.m_potParams[2][3].m_maxValue = 1023;

  memoryService.handleEvent({EventType::kSaveLogicalState, 0, {}});
}

void test_boot_sequence() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  PresetService presetService(logicalState);
  ProgramService programService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MenuService menuService(logicalState);

  Service* services[] = {
    &memoryService,
    &fsmService,
    &presetService,
    &programService,
    &bypassService,
    &tapService,
    &potService,
    &exprService,
    &tempoService,
    &fv1Service,
    &menuService
  };

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  for (auto* service: services) {
    service->init();
  }

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  // Preset bank loaded from memory Initialization
  TEST_ASSERT_EQUAL(EventType::kPresetBankLoaded, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  // State changed event on FSM initialization
  TEST_ASSERT_EQUAL(EventType::kStateChanged, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  // Menu view event on first screen redraw
  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // LogicalState values
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);
  TEST_ASSERT_EQUAL(2, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(2, logicalState.m_currentPreset);
  // TEST_ASSERT_EQUAL(5, logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_interval);
  TEST_ASSERT_EQUAL(256, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(256, logicalState.m_tempo);
  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[logicalState.m_currentProgram].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kPot2, logicalState.m_exprParams[logicalState.m_currentProgram].m_mappedPot);
  TEST_ASSERT_EQUAL(Direction::kNormal, logicalState.m_exprParams[logicalState.m_currentProgram].m_direction);
  TEST_ASSERT_EQUAL(256, logicalState.m_exprParams[logicalState.m_currentProgram].m_heelValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_exprParams[logicalState.m_currentProgram].m_toeValue);

  // ProgramService sets the pointer to the active program
  TEST_ASSERT_EQUAL("Oil-can delay", logicalState.m_activeProgram->m_name);

  // BypassService switches the bypass
  TEST_ASSERT_EQUAL(1, mockBypass.m_kState);
  TEST_ASSERT_EQUAL(1, mockBypass.m_okState);

  // Fv1Service sets the S0/1/2 pins
  TEST_ASSERT_EQUAL(0, mockFv1.m_s0);
  TEST_ASSERT_EQUAL(1, mockFv1.m_s1);
  TEST_ASSERT_EQUAL(0, mockFv1.m_s2);

  // And P0/1/2
  TEST_ASSERT_FALSE(mockFv1.m_potValues.empty());
  auto [pot2, val2] = mockFv1.m_potValues.back();
  TEST_ASSERT_EQUAL(Fv1Pot::Pot2, pot2);
  TEST_ASSERT_EQUAL(512, val2);
  mockFv1.m_potValues.pop_back();

  TEST_ASSERT_FALSE(mockFv1.m_potValues.empty());
  auto [pot1, val1] = mockFv1.m_potValues.back();
  TEST_ASSERT_EQUAL(Fv1Pot::Pot1, pot1);
  TEST_ASSERT_EQUAL(300, val1);
  mockFv1.m_potValues.pop_back();

  TEST_ASSERT_FALSE(mockFv1.m_potValues.empty());
  auto [pot0, val0] = mockFv1.m_potValues.back();
  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, pot0);
  TEST_ASSERT_EQUAL(613, val0);
  mockFv1.m_potValues.pop_back();

  TEST_ASSERT_TRUE(mockFv1.m_potValues.empty());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_boot_sequence);
  UNITY_END();
}
