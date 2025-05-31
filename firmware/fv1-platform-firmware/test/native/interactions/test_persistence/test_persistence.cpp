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
#include "services/program_mode_service.h"
#include "services/program_service.h"
#include "services/tap_service.h"
#include "services/tempo_service.h"
#include "mock/mock_eeprom.h"
#include "mock/mock_fv1.h"
#include "mock/mock_bypass.h"
#include "mock/mock_clock.h"

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
#include "../src/services/program_mode_service.cpp"
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
  logicalState.m_currentProgram = 3;
  logicalState.m_currentPreset = 2;
  logicalState.m_currentPresetBank = 5;
  logicalState.m_midiChannel = 7;
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 512;
  logicalState.m_divInterval = 256;
  logicalState.m_tempo = 256;
  logicalState.m_exprParams[1].m_state = ExprState::kInactive;
  logicalState.m_exprParams[1].m_mappedPot = MappedPot::kPot1;
  logicalState.m_exprParams[1].m_direction = Direction::kInverted;
  logicalState.m_exprParams[1].m_heelValue = 256;
  logicalState.m_exprParams[1].m_toeValue = 512;
  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot2;
  logicalState.m_exprParams[2].m_direction = Direction::kNormal;
  logicalState.m_exprParams[2].m_heelValue = 256;
  logicalState.m_exprParams[2].m_toeValue = 512;
  logicalState.m_exprParams[3].m_state = ExprState::kActive;
  logicalState.m_exprParams[3].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[3].m_direction = Direction::kInverted;
  logicalState.m_exprParams[3].m_heelValue = 0;
  logicalState.m_exprParams[3].m_toeValue = 1023;
  logicalState.m_potParams[1][0].m_state = PotState::kActive;
  logicalState.m_potParams[1][0].m_value = 350;
  logicalState.m_potParams[1][0].m_minValue = 0;
  logicalState.m_potParams[1][0].m_maxValue = 512;
  logicalState.m_potParams[1][1].m_state = PotState::kActive;
  logicalState.m_potParams[1][1].m_value = 300;
  logicalState.m_potParams[1][1].m_minValue = 256;
  logicalState.m_potParams[1][1].m_maxValue = 1023;
  logicalState.m_potParams[1][2].m_state = PotState::kActive;
  logicalState.m_potParams[1][2].m_value = 512;
  logicalState.m_potParams[1][2].m_minValue = 0;
  logicalState.m_potParams[1][2].m_maxValue = 512;
  logicalState.m_potParams[1][3].m_state = PotState::kActive;
  logicalState.m_potParams[1][3].m_value = 256;
  logicalState.m_potParams[1][3].m_minValue = 0;
  logicalState.m_potParams[1][3].m_maxValue = 256;
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
  logicalState.m_potParams[3][0].m_state = PotState::kActive;
  logicalState.m_potParams[3][0].m_value = 300;
  logicalState.m_potParams[3][0].m_minValue = 256;
  logicalState.m_potParams[3][0].m_maxValue = 512;
  logicalState.m_potParams[3][1].m_state = PotState::kActive;
  logicalState.m_potParams[3][1].m_value = 300;
  logicalState.m_potParams[3][1].m_minValue = 0;
  logicalState.m_potParams[3][1].m_maxValue = 1023;
  logicalState.m_potParams[3][2].m_state = PotState::kActive;
  logicalState.m_potParams[3][2].m_value = 512;
  logicalState.m_potParams[3][2].m_minValue = 0;
  logicalState.m_potParams[3][2].m_maxValue = 1023;
  logicalState.m_potParams[3][3].m_state = PotState::kActive;
  logicalState.m_potParams[3][3].m_value = 256;
  logicalState.m_potParams[3][3].m_minValue = 0;
  logicalState.m_potParams[3][3].m_maxValue = 1023;

  memoryService.handleEvent({EventType::kSaveLogicalState, 0, {}});

  auto& bank = memoryService.getLoadedBank();

  bank.m_presets[2].m_programIndex = 1;
  bank.m_presets[2].m_tapState = TapState::kEnabled;
  bank.m_presets[2].m_divState = DivState::kDisabled;
  bank.m_presets[2].m_divValue = DivValue::kEightTriplet;
  bank.m_presets[2].m_interval = 300;
  bank.m_presets[2].m_divInterval = 0;
  bank.m_presets[2].m_tempo = 300;
  bank.m_presets[2].m_exprState = ExprState::kActive;
  bank.m_presets[2].m_mappedPot = MappedPot::kPot2;
  bank.m_presets[2].m_direction = Direction::kInverted;
  bank.m_presets[2].m_heelValue = 100;
  bank.m_presets[2].m_toeValue = 800;
  bank.m_presets[2].m_potParams[0].m_state = PotState::kActive;
  bank.m_presets[2].m_potParams[0].m_value = 50;
  bank.m_presets[2].m_potParams[0].m_minValue = 0;
  bank.m_presets[2].m_potParams[0].m_maxValue = 1023;
  bank.m_presets[2].m_potParams[1].m_state = PotState::kActive;
  bank.m_presets[2].m_potParams[1].m_value = 512;
  bank.m_presets[2].m_potParams[1].m_minValue = 256;
  bank.m_presets[2].m_potParams[1].m_maxValue = 768;
  bank.m_presets[2].m_potParams[2].m_state = PotState::kActive;
  bank.m_presets[2].m_potParams[2].m_value = 100;
  bank.m_presets[2].m_potParams[2].m_minValue = 100;
  bank.m_presets[2].m_potParams[2].m_maxValue = 800;
  bank.m_presets[2].m_potParams[3].m_state = PotState::kActive;
  bank.m_presets[2].m_potParams[3].m_value = 0;
  bank.m_presets[2].m_potParams[3].m_minValue = 0;
  bank.m_presets[2].m_potParams[3].m_maxValue = 1023;

  uint16_t value = 0;
  Utils::unpack16(logicalState.m_currentPreset, logicalState.m_currentPresetBank, value);
  memoryService.handleEvent({EventType::kSavePreset, 0, {.value=value}});

  logicalState.m_currentPreset = 3;

  bank.m_presets[3].m_programIndex = 6;
  bank.m_presets[3].m_tapState = TapState::kDisabled;
  bank.m_presets[3].m_divState = DivState::kDisabled;
  bank.m_presets[3].m_divValue = DivValue::kQuarter;
  bank.m_presets[3].m_interval = 0;
  bank.m_presets[3].m_divInterval = 0;
  bank.m_presets[3].m_tempo = 0;
  bank.m_presets[3].m_exprState = ExprState::kInactive;
  bank.m_presets[3].m_mappedPot = MappedPot::kPot0;
  bank.m_presets[3].m_direction = Direction::kInverted;
  bank.m_presets[3].m_heelValue = 0;
  bank.m_presets[3].m_toeValue = 1023;
  bank.m_presets[3].m_potParams[0].m_state = PotState::kActive;
  bank.m_presets[3].m_potParams[0].m_value = 500;
  bank.m_presets[3].m_potParams[0].m_minValue = 0;
  bank.m_presets[3].m_potParams[0].m_maxValue = 1023;
  bank.m_presets[3].m_potParams[1].m_state = PotState::kActive;
  bank.m_presets[3].m_potParams[1].m_value = 128;
  bank.m_presets[3].m_potParams[1].m_minValue = 96;
  bank.m_presets[3].m_potParams[1].m_maxValue = 500;
  bank.m_presets[3].m_potParams[2].m_state = PotState::kActive;
  bank.m_presets[3].m_potParams[2].m_value = 250;
  bank.m_presets[3].m_potParams[2].m_minValue = 200;
  bank.m_presets[3].m_potParams[2].m_maxValue = 700;
  bank.m_presets[3].m_potParams[3].m_state = PotState::kActive;
  bank.m_presets[3].m_potParams[3].m_value = 0;
  bank.m_presets[3].m_potParams[3].m_minValue = 0;
  bank.m_presets[3].m_potParams[3].m_maxValue = 1023;

  Utils::unpack16(logicalState.m_currentPreset, logicalState.m_currentPresetBank, value);
  memoryService.handleEvent({EventType::kSavePreset, 0, {.value=value}});
}

void runEventChain(Service* services[], size_t count) {
  while (EventBus::hasEvent()) {
    Event e;
    EventBus::recall(e);

    for (size_t i = 0; i < count; ++i) {
      if (services[i]->interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type))) {
        services[i]->handleEvent(e);
      }
    }
  }
}

void runUpdateChain(Service* services[], size_t count) {
  for (size_t i = 0; i < count; ++i) {
    services[i]->update();
  }
}

void test_bypass() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;
  MockedClock clock;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  ProgramModeService programModeService(logicalState);
  ProgramService programService(logicalState);
  PresetService presetService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MenuService menuService(logicalState, clock);

  Service* services[] = {
    &memoryService,
    &fsmService,
    &programModeService,
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

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  EventBus::publish({EventType::kBootCompleted});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  EventBus::publish({EventType::kRawBypassPressed});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_bypassState>(mockEeprom.m_memory));

  EventBus::publish({EventType::kRawBypassPressed});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_bypassState>(mockEeprom.m_memory));
}

void test_program_mode() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;
  MockedClock clock;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  ProgramModeService programModeService(logicalState);
  ProgramService programService(logicalState);
  PresetService presetService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MenuService menuService(logicalState, clock);

  Service* services[] = {
    &memoryService,
    &fsmService,
    &programModeService,
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

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  EventBus::publish({EventType::kBootCompleted});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  mockFv1.m_potValues.clear();

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  EventBus::publish({EventType::kRawProgramModeSwitchLongPress});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_programMode>(mockEeprom.m_memory));

  EventBus::publish({EventType::kRawProgramModeSwitchLongPress});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_programMode>(mockEeprom.m_memory));
}

void test_current_program() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;
  MockedClock clock;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  ProgramModeService programModeService(logicalState);
  ProgramService programService(logicalState);
  PresetService presetService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MenuService menuService(logicalState, clock);

  Service* services[] = {
    &memoryService,
    &fsmService,
    &programModeService,
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

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  EventBus::publish({EventType::kBootCompleted});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  mockFv1.m_potValues.clear();

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(3, logicalState.m_currentProgram);

  EventBus::publish({EventType::kMenuProgramChanged, 0, {.delta=1}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(4, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(4, std::get<MemoryLayout::c_currentProgram>(mockEeprom.m_memory));

  EventBus::publish({EventType::kMenuProgramChanged, 0, {.delta=1}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(5, std::get<MemoryLayout::c_currentProgram>(mockEeprom.m_memory));

  EventBus::publish({EventType::kMenuProgramChanged, 0, {.delta=-1}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(4, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(4, std::get<MemoryLayout::c_currentProgram>(mockEeprom.m_memory));
}

void test_current_preset_preset_bank() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;
  MockedClock clock;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  ProgramModeService programModeService(logicalState);
  ProgramService programService(logicalState);
  PresetService presetService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MenuService menuService(logicalState, clock);

  Service* services[] = {
    &memoryService,
    &fsmService,
    &programModeService,
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

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  EventBus::publish({EventType::kBootCompleted});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  mockFv1.m_potValues.clear();

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  EventBus::publish({EventType::kRawProgramModeSwitchLongPress});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
  TEST_ASSERT_EQUAL(2, logicalState.m_currentPreset);

  EventBus::publish({EventType::kMenuPresetChanged, 0, {.delta=-1}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(1, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_currentPreset>(mockEeprom.m_memory));

  EventBus::publish({EventType::kMenuPresetBankChanged, 0, {.delta=1}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(6, logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(6, std::get<MemoryLayout::c_currentPresetBank>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_currentPreset>(mockEeprom.m_memory));
}

void test_midi_channel() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;
  MockedClock clock;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  ProgramModeService programModeService(logicalState);
  ProgramService programService(logicalState);
  PresetService presetService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MidiService midiService(logicalState);
  MenuService menuService(logicalState, clock);

  Service* services[] = {
    &memoryService,
    &fsmService,
    &programModeService,
    &presetService,
    &programService,
    &bypassService,
    &tapService,
    &potService,
    &exprService,
    &tempoService,
    &fv1Service,
    &midiService,
    &menuService
  };

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  for (auto* service: services) {
    service->init();
  }

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  EventBus::publish({EventType::kBootCompleted});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  mockFv1.m_potValues.clear();

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  EventBus::publish({EventType::kMenuMidiChannelChanged, 0, {.delta=-1}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(6, logicalState.m_midiChannel);
  TEST_ASSERT_EQUAL(6, std::get<MemoryLayout::c_midiChannel>(mockEeprom.m_memory));
}

void test_tap() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;
  MockedClock clock;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  ProgramModeService programModeService(logicalState);
  ProgramService programService(logicalState);
  PresetService presetService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MidiService midiService(logicalState);
  MenuService menuService(logicalState, clock);

  std::get<MemoryLayout::c_divValue>(mockEeprom.m_memory) = 0; // Reset divvalue

  Service* services[] = {
    &memoryService,
    &fsmService,
    &programModeService,
    &presetService,
    &programService,
    &bypassService,
    &tapService,
    &potService,
    &exprService,
    &tempoService,
    &fv1Service,
    &midiService,
    &menuService
  };

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  for (auto* service: services) {
    service->init();
  }

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  EventBus::publish({EventType::kBootCompleted});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  mockFv1.m_potValues.clear();

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  logicalState.m_tapState = TapState::kDisabled;
  logicalState.m_divState = DivState::kDisabled;
  logicalState.m_divValue = DivValue::kQuarter;
  logicalState.m_interval = 0;
  logicalState.m_divInterval = 0;

  EventBus::publish({EventType::kRawTapPressed, 200, {}});
  EventBus::publish({EventType::kRawTapPressed, 600, {}});
  EventBus::publish({EventType::kRawTapPressed, 1000, {}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(400, logicalState.m_interval);
  TEST_ASSERT_EQUAL(0, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_tapState>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_divState>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_divValue>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(144, std::get<MemoryLayout::c_intervalL>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_intervalH>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_divIntervalL>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_divIntervalH>(mockEeprom.m_memory));

  EventBus::publish({EventType::kRawTapLongPressed, 1200, {}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(400, logicalState.m_interval);
  TEST_ASSERT_EQUAL(200, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_tapState>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_divState>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_divValue>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(144, std::get<MemoryLayout::c_intervalL>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_intervalH>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(200, std::get<MemoryLayout::c_divIntervalL>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_divIntervalH>(mockEeprom.m_memory));

  EventBus::publish({EventType::kPot0Moved, 1400, {.value=150}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(400, logicalState.m_interval);
  TEST_ASSERT_EQUAL(200, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_tapState>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_divState>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_divValue>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(144, std::get<MemoryLayout::c_intervalL>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_intervalH>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(200, std::get<MemoryLayout::c_divIntervalL>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<MemoryLayout::c_divIntervalH>(mockEeprom.m_memory));
}

void test_tempo() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;
  MockedClock clock;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  ProgramModeService programModeService(logicalState);
  ProgramService programService(logicalState);
  PresetService presetService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MidiService midiService(logicalState);
  MenuService menuService(logicalState, clock);

  std::get<MemoryLayout::c_divValue>(mockEeprom.m_memory) = 0; // Reset divvalue

  Service* services[] = {
    &memoryService,
    &fsmService,
    &programModeService,
    &presetService,
    &programService,
    &bypassService,
    &tapService,
    &potService,
    &exprService,
    &tempoService,
    &fv1Service,
    &midiService,
    &menuService
  };

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  for (auto* service: services) {
    service->init();
  }

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  EventBus::publish({EventType::kBootCompleted});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  mockFv1.m_potValues.clear();

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  logicalState.m_tapState = TapState::kDisabled;
  logicalState.m_divState = DivState::kDisabled;
  logicalState.m_divValue = DivValue::kQuarter;
  logicalState.m_interval = 0;
  logicalState.m_divInterval = 0;

  EventBus::publish({EventType::kRawTapPressed, 200, {}});
  EventBus::publish({EventType::kRawTapPressed, 600, {}});
  EventBus::publish({EventType::kRawTapPressed, 1000, {}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(400, logicalState.m_tempo);
  TEST_ASSERT_EQUAL(144, std::get<MemoryLayout::c_tempoL>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(1, std::get<MemoryLayout::c_tempoH>(mockEeprom.m_memory));
}

void test_expr() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;
  MockedClock clock;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  ProgramModeService programModeService(logicalState);
  ProgramService programService(logicalState);
  PresetService presetService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MidiService midiService(logicalState);
  MenuService menuService(logicalState, clock);

  Service* services[] = {
    &memoryService,
    &fsmService,
    &programModeService,
    &presetService,
    &programService,
    &bypassService,
    &tapService,
    &potService,
    &exprService,
    &tempoService,
    &fv1Service,
    &midiService,
    &menuService
  };

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  for (auto* service: services) {
    service->init();
  }

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  EventBus::publish({EventType::kBootCompleted});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  mockFv1.m_potValues.clear();

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  EventBus::publish({EventType::kMenuExprStateToggled, 0, {}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(ExprState::kInactive, logicalState.m_exprParams[logicalState.m_currentProgram].m_state);
  constexpr uint16_t exprAddress = MemoryLayout::c_exprStart + (MemoryLayout::c_exprSize * 3);
  TEST_ASSERT_EQUAL(0, std::get<exprAddress>(mockEeprom.m_memory));

  EventBus::publish({EventType::kMenuExprMappedPotMoved, 0, {.delta=1}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(MappedPot::kPot1, logicalState.m_exprParams[logicalState.m_currentProgram].m_mappedPot);
  TEST_ASSERT_EQUAL(1, std::get<exprAddress+1>(mockEeprom.m_memory));

  EventBus::publish({EventType::kMenuExprDirectionToggled, 0, {}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(Direction::kNormal, logicalState.m_exprParams[logicalState.m_currentProgram].m_direction);
  TEST_ASSERT_EQUAL(0, std::get<exprAddress+2>(mockEeprom.m_memory));

  EventBus::publish({EventType::kMenuExprHeelValueMoved, 0, {.delta=1}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(1, logicalState.m_exprParams[logicalState.m_currentProgram].m_heelValue);
  TEST_ASSERT_EQUAL(1, std::get<exprAddress+3>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(0, std::get<exprAddress+4>(mockEeprom.m_memory));

  EventBus::publish({EventType::kMenuExprToeValueMoved, 0, {.delta=-1}});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(1022, logicalState.m_exprParams[logicalState.m_currentProgram].m_toeValue);
  TEST_ASSERT_EQUAL(254, std::get<exprAddress+5>(mockEeprom.m_memory));
  TEST_ASSERT_EQUAL(3, std::get<exprAddress+6>(mockEeprom.m_memory));
}

void test_pot() {
  fillMockEeprom();

  LogicalState logicalState;

  MockEEPROM mockEeprom;
  MockFv1 mockFv1;
  MockBypass mockBypass;
  MockedClock clock;

  FsmService fsmService(logicalState);
  MemoryService memoryService(logicalState, mockEeprom);
  ProgramModeService programModeService(logicalState);
  ProgramService programService(logicalState);
  PresetService presetService(logicalState);
  BypassService bypassService(logicalState, mockBypass);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  Fv1Service fv1Service(logicalState, mockFv1);
  MidiService midiService(logicalState);
  MenuService menuService(logicalState, clock);

  Service* services[] = {
    &memoryService,
    &fsmService,
    &programModeService,
    &presetService,
    &programService,
    &bypassService,
    &tapService,
    &potService,
    &exprService,
    &tempoService,
    &fv1Service,
    &midiService,
    &menuService
  };

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  for (auto* service: services) {
    service->init();
  }

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  EventBus::publish({EventType::kBootCompleted});

  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  mockFv1.m_potValues.clear();

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    EventType event = static_cast<EventType>(static_cast<uint8_t>(EventType::kMenuPot0StateToggled) + i);
    EventBus::publish({event, 0, {}});

    runEventChain(services, servicesCount);
    runUpdateChain(services, servicesCount);

    TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[logicalState.m_currentProgram][i].m_state);
    size_t address = MemoryLayout::getPotParamOffset(logicalState.m_currentProgram, i);
    TEST_ASSERT_EQUAL(0, mockEeprom.m_memory.at(address));
  }

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    EventType event = static_cast<EventType>(static_cast<uint8_t>(EventType::kMenuPot0MinValueMoved) + i);
    uint16_t value = logicalState.m_potParams[logicalState.m_currentProgram][i].m_minValue + 1;
    EventBus::publish({event, 0, {.delta=1}});

    runEventChain(services, servicesCount);
    runUpdateChain(services, servicesCount);

    size_t address = MemoryLayout::getPotParamOffset(logicalState.m_currentProgram, i);
    uint16_t newvalue = 0;
    Utils::unpack16(mockEeprom.m_memory.at(address + 3), mockEeprom.m_memory.at(address + 4), newvalue);

    TEST_ASSERT_EQUAL(value, newvalue);
  }

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    EventType event = static_cast<EventType>(static_cast<uint8_t>(EventType::kMenuPot0MaxValueMoved) + i);
    uint16_t value = logicalState.m_potParams[logicalState.m_currentProgram][i].m_maxValue - 1;
    EventBus::publish({event, 0, {.delta=-1}});

    runEventChain(services, servicesCount);
    runUpdateChain(services, servicesCount);

    size_t address = MemoryLayout::getPotParamOffset(logicalState.m_currentProgram, i);
    uint16_t newvalue = 0;
    Utils::unpack16(mockEeprom.m_memory.at(address + 5), mockEeprom.m_memory.at(address + 6), newvalue);

    TEST_ASSERT_EQUAL(value, newvalue);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_bypass);
  RUN_TEST(test_program_mode);
  RUN_TEST(test_current_program);
  RUN_TEST(test_current_preset_preset_bank);
  RUN_TEST(test_midi_channel);
  RUN_TEST(test_tap);
  RUN_TEST(test_tempo);
  RUN_TEST(test_expr);
  RUN_TEST(test_pot);
  UNITY_END();
}
