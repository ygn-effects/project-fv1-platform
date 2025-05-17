#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/bypass_service.h"
#include "services/expr_service.h"
#include "services/fsm_service.h"
#include "services/menu_service.h"
#include "services/pot_service.h"
#include "services/program_service.h"
#include "services/tap_service.h"
#include "services/tempo_service.h"
#include "services/midi_service.h"

#include "../src/logic/pot_handler.cpp"
#include "../src/logic/tap_handler.cpp"
#include "../src/logic/expr_handler.cpp"
#include "../src/logic/midi_handler.cpp"
#include "../src/services/bypass_service.cpp"
#include "../src/services/expr_service.cpp"
#include "../src/services/fsm_service.cpp"
#include "../src/services/menu_service.cpp"
#include "../src/services/pot_service.cpp"
#include "../src/services/program_service.cpp"
#include "../src/services/tap_service.cpp"
#include "../src/services/tempo_service.cpp"
#include "../src/services/midi_service.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

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

void test() {
  LogicalState logicalState;

  FsmService fsmService(logicalState);
  BypassService bypassService(logicalState);
  ProgramService programService(logicalState);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  MenuService menuService(logicalState);

  Service* services[] = {
    &fsmService,
    &bypassService,
    &programService,
    &potService,
    &exprService,
    &tapService,
    &tempoService,
    &menuService
  };

  for (auto* service: services) {
    service->init();
  }

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  EventBus::publish({EventType::kBootCompleted, 10, {}});

  // Bypass foot switch press
  EventBus::publish({EventType::kRawBypassPressed, 100, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Bypass is off
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);

  // Bypass foot switch press
  EventBus::publish({EventType::kRawBypassPressed, 100, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Bypass is on
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  // Menu is locked
  TEST_ASSERT_EQUAL("Lock screen", menuService.getMenuView()->m_header);

  // Unlock menu
  EventBus::publish({EventType::kRawMenuEncoderLongPressed, 2000, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Menu is unlocked
  TEST_ASSERT_EQUAL("Program mode", menuService.getMenuView()->m_header);

  // Move pots
  EventBus::publish({EventType::kRawPot0Moved, 3000, {.value=509}});
  EventBus::publish({EventType::kRawPot1Moved, 3000, {.value=510}});
  EventBus::publish({EventType::kRawPot2Moved, 3000, {.value=511}});
  EventBus::publish({EventType::kRawMixPotMoved, 3000, {.value=512}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Assert pot movement
  TEST_ASSERT_EQUAL(509, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(510, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(511, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);

  if (logicalState.m_activeProgram->m_isDelayEffect) {
    // Assert pot movement reflected on the tempo
    TEST_ASSERT_EQUAL(507, logicalState.m_tempo);

    // Assert menu composition
    TEST_ASSERT_EQUAL("Program mode", menuService.getMenuView()->m_header);
    TEST_ASSERT_EQUAL("Prog", menuService.getMenuView()->m_items[0]->m_label(&logicalState));
    TEST_ASSERT_EQUAL("Tempo", menuService.getMenuView()->m_items[1]->m_label(&logicalState));
    TEST_ASSERT_EQUAL("Feedback", menuService.getMenuView()->m_items[2]->m_label(&logicalState));
    TEST_ASSERT_EQUAL("Tone LPF", menuService.getMenuView()->m_items[3]->m_label(&logicalState));
    TEST_ASSERT_EQUAL("Mix", menuService.getMenuView()->m_items[4]->m_label(&logicalState));

    // Assert cursor location
    TEST_ASSERT_EQUAL(0, menuService.getMenuView()->m_selected);
    TEST_ASSERT_EQUAL("Prog", menuService.getMenuView()->m_items[menuService.getMenuView()->m_selected]->m_label(&logicalState));
  }

  // Move encoder down
  EventBus::publish({EventType::kRawMenuEncoderMoved, 3100, {.delta=1}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Assert cursor position and current select item
  TEST_ASSERT_EQUAL(1, menuService.getMenuView()->m_selected);
  TEST_ASSERT_EQUAL("Tempo", menuService.getMenuView()->m_items[menuService.getMenuView()->m_selected]->m_label(&logicalState));

  // Begin edit
  EventBus::publish({EventType::kRawMenuEncoderPressed, 3200, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  // Change value
  EventBus::publish({EventType::kRawMenuEncoderMoved, 3300, {.delta=10}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Assert tempo change
  TEST_ASSERT_EQUAL(517, logicalState.m_tempo);

  // End edit
  EventBus::publish({EventType::kMenuEncoderPressed, 3400, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  // Move encoder down
  EventBus::publish({EventType::kRawMenuEncoderMoved, 3500, {.delta=1}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Assert cursor position and current select item
  TEST_ASSERT_EQUAL(2, menuService.getMenuView()->m_selected);
  TEST_ASSERT_EQUAL("Feedback", menuService.getMenuView()->m_items[menuService.getMenuView()->m_selected]->m_label(&logicalState));

  // Begin edit
  EventBus::publish({EventType::kRawMenuEncoderPressed, 3200, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  // Change value
  EventBus::publish({EventType::kRawMenuEncoderMoved, 3300, {.delta=10}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Assert feedback change
  TEST_ASSERT_EQUAL(520, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);

  // End edit
  EventBus::publish({EventType::kMenuEncoderPressed, 3400, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  // Move encoder down
  EventBus::publish({EventType::kRawMenuEncoderMoved, 3600, {.delta=1}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Assert cursor position and current select item
  TEST_ASSERT_EQUAL(3, menuService.getMenuView()->m_selected);
  TEST_ASSERT_EQUAL("Tone LPF", menuService.getMenuView()->m_items[menuService.getMenuView()->m_selected]->m_label(&logicalState));

  // Begin edit
  EventBus::publish({EventType::kRawMenuEncoderPressed, 3700, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  // Change value
  EventBus::publish({EventType::kRawMenuEncoderMoved, 3800, {.delta=10}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Assert low pass change
  TEST_ASSERT_EQUAL(521, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);

  // End edit
  EventBus::publish({EventType::kMenuEncoderPressed, 3400, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  // Move encoder down
  EventBus::publish({EventType::kRawMenuEncoderMoved, 3600, {.delta=1}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Assert cursor position and current select item
  TEST_ASSERT_EQUAL(4, menuService.getMenuView()->m_selected);
  TEST_ASSERT_EQUAL("Mix", menuService.getMenuView()->m_items[menuService.getMenuView()->m_selected]->m_label(&logicalState));

  // Begin edit
  EventBus::publish({EventType::kRawMenuEncoderPressed, 3700, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  // Change value
  EventBus::publish({EventType::kRawMenuEncoderMoved, 3800, {.delta=10}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Assert low pass change
  TEST_ASSERT_EQUAL(522, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);

  // End edit
  EventBus::publish({EventType::kMenuEncoderPressed, 3400, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  // Move encoder down
  EventBus::publish({EventType::kRawMenuEncoderMoved, 3600, {.delta=1}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  if (logicalState.m_activeProgram->m_isDelayEffect) {
    // Assert menu composition
    TEST_ASSERT_EQUAL("Program mode", menuService.getMenuView()->m_header);
    TEST_ASSERT_EQUAL("Tempo", menuService.getMenuView()->m_items[0]->m_label(&logicalState));
    TEST_ASSERT_EQUAL("Feedback", menuService.getMenuView()->m_items[1]->m_label(&logicalState));
    TEST_ASSERT_EQUAL("Tone LPF", menuService.getMenuView()->m_items[2]->m_label(&logicalState));
    TEST_ASSERT_EQUAL("Mix", menuService.getMenuView()->m_items[3]->m_label(&logicalState));
    TEST_ASSERT_EQUAL("Expression settings", menuService.getMenuView()->m_items[4]->m_label(&logicalState));

    // Assert cursor location
    TEST_ASSERT_EQUAL(4, menuService.getMenuView()->m_selected);
    TEST_ASSERT_EQUAL("Expression settings", menuService.getMenuView()->m_items[menuService.getMenuView()->m_selected]->m_label(&logicalState));
  }
}

void test_midi() {
  LogicalState logicalState;

  FsmService fsmService(logicalState);
  MidiService midiService(logicalState);
  BypassService bypassService(logicalState);
  ProgramService programService(logicalState);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  MenuService menuService(logicalState);

  MidiHandler* midiHandler = midiService.getMidiHandler();

  Service* services[] = {
    &fsmService,
    &midiService,
    &bypassService,
    &programService,
    &potService,
    &exprService,
    &tapService,
    &tempoService,
    &menuService
  };

  for (auto* service: services) {
    service->init();
  }

  uint8_t servicesCount = sizeof(services) / sizeof(services[0]);

  EventBus::publish({EventType::kBootCompleted, 10, {}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Bypass is on
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x04);
  midiHandler->pushByte(0x00);
  runUpdateChain(services, servicesCount);
  runEventChain(services, servicesCount);

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Bypass is off
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x04);
  midiHandler->pushByte(0x7F);
  runUpdateChain(services, servicesCount);
  runEventChain(services, servicesCount);

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Bypass is on
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x00);
  midiHandler->pushByte(0x3E);
  runUpdateChain(services, servicesCount);
  runEventChain(services, servicesCount);

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Bypass is on
  TEST_ASSERT_EQUAL(499, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x01);
  midiHandler->pushByte(0x20);
  runUpdateChain(services, servicesCount);
  runEventChain(services, servicesCount);

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Bypass is on
  TEST_ASSERT_EQUAL(257, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x02);
  midiHandler->pushByte(0x10);
  runUpdateChain(services, servicesCount);
  runEventChain(services, servicesCount);

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Bypass is on
  TEST_ASSERT_EQUAL(128, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);

  midiHandler->pushByte(0xB0);
  midiHandler->pushByte(0x03);
  midiHandler->pushByte(0x05);
  runUpdateChain(services, servicesCount);
  runEventChain(services, servicesCount);

  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Bypass is on
  TEST_ASSERT_EQUAL(40, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);

  // Tap is disabled
  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);

  tapService.handleEvent({EventType::kMidiTapPressed, 200, {.value=0}});
  tapService.handleEvent({EventType::kMidiTapPressed, 400, {.value=0}});
  tapService.handleEvent({EventType::kMidiTapPressed, 600, {.value=0}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Tap is activated
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  // Assert tempo
  TEST_ASSERT_EQUAL(200, logicalState.m_tempo);

  // Div is disabled
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);

  tapService.handleEvent({EventType::kMidiTapPressed, 200, {.value=127}});
  runEventChain(services, servicesCount);
  runUpdateChain(services, servicesCount);

  // Div is enabled
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test);
  RUN_TEST(test_midi);
  UNITY_END();
}