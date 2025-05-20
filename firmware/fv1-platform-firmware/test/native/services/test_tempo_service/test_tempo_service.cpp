#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/tempo_service.h"
#include "services/tap_service.h"
#include "services/program_service.h"

#include "../src/services/tempo_service.cpp"
#include "../src/services/tap_service.cpp"
#include "../src/logic/tap_handler.cpp"
#include "../src/services/program_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_tap_tempo() {
  LogicalState logicalState;
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);

  tapService.init();
  tempoService.init();

  tapService.handleEvent({EventType::kTapPressed, 200, {}});
  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);

  tapService.handleEvent({EventType::kTapPressed, 400, {}});
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event event;
  EventBus::recall(event);

  tempoService.handleEvent(event);
  TEST_ASSERT_EQUAL(200, logicalState.m_tempo);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(EventType::kTempoChanged, event.m_type);
  TEST_ASSERT_EQUAL(200, event.m_data.value);
}

void test_pot() {
  LogicalState logicalState;
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);

  tapService.init();
  tempoService.init();

  tempoService.handleEvent({EventType::kPot0Moved, 1000, {.value = 200}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event event;
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(211, logicalState.m_tempo);
  TEST_ASSERT_EQUAL(EventType::kTempoChanged, event.m_type);
  TEST_ASSERT_EQUAL(211, event.m_data.value);
}

void test_pot_map() {
  LogicalState logicalState;
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);

  tapService.init();
  tempoService.init();

  tempoService.handleEvent({EventType::kPot0Moved, 1000, {.value = 0}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event event;
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(20, logicalState.m_tempo);

  tempoService.handleEvent({EventType::kPot0Moved, 1000, {.value = 1023}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(1000, logicalState.m_tempo);
}

void test_program_change() {
  LogicalState logicalState;
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  ProgramService programService(logicalState);

  tapService.init();
  tempoService.init();

  tempoService.handleEvent({EventType::kPot0Moved, 1000, {.value = 1023}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event event;
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(1000, logicalState.m_tempo);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = 1}});
  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);

  tempoService.handleEvent({EventType::kProgramChanged, 1000, {.delta = 1}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(800, logicalState.m_tempo);
}

void test_interested_in() {
  LogicalState logicalState;
  TempoService tempoService(logicalState);

  Event e{EventType::kTapIntervalChanged, 500, {.value=100}};
  TEST_ASSERT_TRUE(tempoService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kProgramChanged, 500, {.delta=1}};
  TEST_ASSERT_TRUE(tempoService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuTempoChanged, 500, {.delta=1}};
  TEST_ASSERT_TRUE(tempoService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kPot0Moved, 500, {.delta=1}};
  TEST_ASSERT_TRUE(tempoService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kExprMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(tempoService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuExprMappedPotMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(tempoService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_tap_tempo);
  RUN_TEST(test_pot);
  RUN_TEST(test_pot_map);
  RUN_TEST(test_program_change);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
