#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/poll_service.h"

#include "../src/services/poll_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_switch() {
  LogicalState logicalState;
  PollService pollService(logicalState);

  pollService.handleEvent({EventType::kSwitchPressed, 0, {.id=static_cast<uint16_t>(SwitchId::kBypass)}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawBypassPressed, e.m_type);

  pollService.handleEvent({EventType::kSwitchPressed, 0, {.id=static_cast<uint16_t>(SwitchId::kMenuEncoder)}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawMenuEncoderPressed, e.m_type);

  pollService.handleEvent({EventType::kSwitchPressed, 0, {.id=static_cast<uint16_t>(SwitchId::kTap)}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawTapPressed, e.m_type);

  pollService.handleEvent({EventType::kSwitchLongPressed, 0, {.id=static_cast<uint16_t>(SwitchId::kMenuEncoder)}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawMenuEncoderLongPressed, e.m_type);

  pollService.handleEvent({EventType::kSwitchLongPressed, 0, {.id=static_cast<uint16_t>(SwitchId::kTap)}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawTapLongPressed, e.m_type);

  pollService.handleEvent({EventType::kSwitchReleased, 0, {.id=static_cast<uint16_t>(SwitchId::kMenuEncoder)}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawMenuEncoderReleased, e.m_type);

  pollService.handleEvent({EventType::kSwitchReleased, 0, {.id=static_cast<uint16_t>(SwitchId::kTap)}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawTapReleased, e.m_type);
}

void test_encoder() {
  LogicalState logicalState;
  PollService pollService(logicalState);

  pollService.handleEvent({EventType::kEncoderMoved, 0, {.id=static_cast<uint16_t>(EncoderId::kMenuEncoder), .delta=1}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawMenuEncoderMoved, e.m_type);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);

  pollService.handleEvent({EventType::kEncoderMoved, 0, {.id=static_cast<uint16_t>(EncoderId::kMenuEncoder), .delta=-1}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawMenuEncoderMoved, e.m_type);
  TEST_ASSERT_EQUAL(-1, e.m_data.delta);
}

void test_pot() {
  LogicalState logicalState;
  PollService pollService(logicalState);

  pollService.handleEvent({EventType::kPotMoved, 0, {.id=static_cast<uint16_t>(PotId::kPot0), .value=512}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawPot0Moved, e.m_type);
  TEST_ASSERT_EQUAL(512, e.m_data.value);

  pollService.handleEvent({EventType::kPotMoved, 0, {.id=static_cast<uint16_t>(PotId::kPot1), .value=256}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawPot1Moved, e.m_type);
  TEST_ASSERT_EQUAL(256, e.m_data.value);

  pollService.handleEvent({EventType::kPotMoved, 0, {.id=static_cast<uint16_t>(PotId::kPot2), .value=768}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawPot2Moved, e.m_type);
  TEST_ASSERT_EQUAL(768, e.m_data.value);

  pollService.handleEvent({EventType::kPotMoved, 0, {.id=static_cast<uint16_t>(PotId::kMixPot), .value=64}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawMixPotMoved, e.m_type);
  TEST_ASSERT_EQUAL(64, e.m_data.value);

  pollService.handleEvent({EventType::kPotMoved, 0, {.id=static_cast<uint16_t>(PotId::kExpr), .value=96}});

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kRawExprMoved, e.m_type);
  TEST_ASSERT_EQUAL(96, e.m_data.value);
}

void test_interested_in() {
  LogicalState logicalState;
  PollService pollService(logicalState);

  Event e = {EventType::kSwitchPressed, 0, {}};
  TEST_ASSERT_TRUE(pollService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kEncoderMoved, 0, {}};
  TEST_ASSERT_TRUE(pollService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kPotMoved, 0, {}};
  TEST_ASSERT_TRUE(pollService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kPot0Moved, 0, {}};
  TEST_ASSERT_FALSE(pollService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kProgramModeChanged, 0, {}};
  TEST_ASSERT_FALSE(pollService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kProgramChanged, 0, {}};
  TEST_ASSERT_FALSE(pollService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_interested_in);
  RUN_TEST(test_switch);
  RUN_TEST(test_encoder);
  RUN_TEST(test_pot);
  UNITY_END();
}
