#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/tap_service.h"

#include "../src/services/tap_service.cpp"
#include "../src/logic/tap_handler.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_random_event() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  tapService.handleEvent({EventType::kBypassPressed, 0, {}});

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_tap_2_taps() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  tapService.handleEvent({EventType::kTapPressed, 200, {}});
  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);

  tapService.handleEvent({EventType::kTapPressed, 400, {}});
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event tempoChangedEvent;
  EventBus::recall(tempoChangedEvent);

  TEST_ASSERT_EQUAL(EventType::kTapIntervalChanged, tempoChangedEvent.m_type);
  TEST_ASSERT_EQUAL(200, logicalState.m_interval);
  TEST_ASSERT_EQUAL(200, tempoChangedEvent.m_data.value);
}

void test_tap_4_taps() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  tapService.handleEvent({EventType::kTapPressed, 200, {}});
  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  tapService.handleEvent({EventType::kTapPressed, 450, {}});
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event tempoChangedEvent;
  EventBus::recall(tempoChangedEvent);
  TEST_ASSERT_TRUE(EventBus::hasEvent()); // Save tap
  EventBus::recall(tempoChangedEvent);

  tapService.handleEvent({EventType::kTapPressed, 580, {}});
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);
  TEST_ASSERT_TRUE(EventBus::hasEvent()); // Save tap
  EventBus::recall(tempoChangedEvent);

  tapService.handleEvent({EventType::kTapPressed, 800, {}});
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);


  TEST_ASSERT_EQUAL(EventType::kTapIntervalChanged, tempoChangedEvent.m_type);
  TEST_ASSERT_EQUAL(200, logicalState.m_interval);
  TEST_ASSERT_EQUAL(200, tempoChangedEvent.m_data.value);
}

void test_timeout() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  tapService.handleEvent({EventType::kTapPressed, 200, {}});
  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);

  tapService.handleEvent({EventType::kTapPressed, 1201, {}});
  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
}

void test_div() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  tapService.handleEvent({EventType::kTapPressed, 200, {}});
  tapService.handleEvent({EventType::kTapPressed, 400, {}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event tempoChangedEvent;
  EventBus::recall(tempoChangedEvent);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  tapService.handleEvent({EventType::kTapLongPressed, 1000, {}}); // Eight

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  TEST_ASSERT_EQUAL(EventType::kTapIntervalChanged, tempoChangedEvent.m_type);
  TEST_ASSERT_EQUAL(200, logicalState.m_interval);
  TEST_ASSERT_EQUAL(100, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(100, tempoChangedEvent.m_data.value);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  tapService.handleEvent({EventType::kTapLongPressed, 2000, {}}); // Sixteenth

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  TEST_ASSERT_EQUAL(EventType::kTapIntervalChanged, tempoChangedEvent.m_type);
  TEST_ASSERT_EQUAL(200, logicalState.m_interval);
  TEST_ASSERT_EQUAL(50, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(50, tempoChangedEvent.m_data.value);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kSixteenth, logicalState.m_divValue);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  tapService.handleEvent({EventType::kTapLongPressed, 2000, {}}); // Dotted Eight

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  TEST_ASSERT_EQUAL(EventType::kTapIntervalChanged, tempoChangedEvent.m_type);
  TEST_ASSERT_EQUAL(200, logicalState.m_interval);
  TEST_ASSERT_EQUAL(150, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(150, tempoChangedEvent.m_data.value);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kDottedEight, logicalState.m_divValue);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  tapService.handleEvent({EventType::kTapLongPressed, 2000, {}}); // Eight Triplet

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  TEST_ASSERT_EQUAL(EventType::kTapIntervalChanged, tempoChangedEvent.m_type);
  TEST_ASSERT_EQUAL(200, logicalState.m_interval);
  TEST_ASSERT_EQUAL(66, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(66, tempoChangedEvent.m_data.value);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEightTriplet, logicalState.m_divValue);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  tapService.handleEvent({EventType::kTapLongPressed, 2000, {}}); // Wrap around to quarter

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  TEST_ASSERT_EQUAL(EventType::kTapIntervalChanged, tempoChangedEvent.m_type);
  TEST_ASSERT_EQUAL(200, logicalState.m_interval);
  TEST_ASSERT_EQUAL(200, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(200, tempoChangedEvent.m_data.value);
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);
}

void test_event_order() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  tapService.handleEvent({EventType::kTapLongPressed, 0, {}});
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  tapService.handleEvent({EventType::kTapPressed, 200, {}});
  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);

  tapService.handleEvent({EventType::kTapPressed, 400, {}});
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event tempoChangedEvent;
  EventBus::recall(tempoChangedEvent);

  TEST_ASSERT_EQUAL(EventType::kTapIntervalChanged, tempoChangedEvent.m_type);
  TEST_ASSERT_EQUAL(200, logicalState.m_interval);
  TEST_ASSERT_EQUAL(200, tempoChangedEvent.m_data.value);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  tapService.handleEvent({EventType::kTapLongPressed, 1000, {}}); // Eight

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(tempoChangedEvent);

  TEST_ASSERT_EQUAL(EventType::kTapIntervalChanged, tempoChangedEvent.m_type);
  TEST_ASSERT_EQUAL(200, logicalState.m_interval);
  TEST_ASSERT_EQUAL(100, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(100, tempoChangedEvent.m_data.value);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);
}

void test_interested_in() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  Event e{EventType::kTapPressed, 500, {.value=100}};
  TEST_ASSERT_TRUE(tapService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kTapLongPressed, 500, {.delta=1}};
  TEST_ASSERT_TRUE(tapService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuTempoChanged, 500, {.delta=1}};
  TEST_ASSERT_TRUE(tapService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kProgramChanged, 500, {.delta=1}};
  TEST_ASSERT_TRUE(tapService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kPot0Moved, 500, {.delta=1}};
  TEST_ASSERT_TRUE(tapService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kExprMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(tapService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuExprMappedPotMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(tapService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_random_event);
  RUN_TEST(test_tap_2_taps);
  RUN_TEST(test_tap_4_taps);
  RUN_TEST(test_timeout);
  RUN_TEST(test_div);
  RUN_TEST(test_event_order);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
