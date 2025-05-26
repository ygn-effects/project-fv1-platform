#include <unity.h>
#include "core/event_bus.h"
#include "hal/poll_manager.h"
#include "mock/mock_pollable.h"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_register() {
  hal::PollManager pollManager;

  MockPollable mockSwitch;
  MockPollable mockEncoder;

  pollManager.registerDevice(&mockSwitch);
  pollManager.registerDevice(&mockEncoder);

  TEST_ASSERT_EQUAL(2, pollManager.getDevicesCount());
}

void test_poll() {
  hal::PollManager pollManager;

  MockPollable mockSwitch;
  MockPollable mockEncoder;

  mockSwitch.init();
  mockEncoder.init();

  mockSwitch.addEvent({EventType::kRawBypassPressed, 0, {}});
  mockSwitch.addEvent({EventType::kRawTapLongPressed, 0, {}});

  mockEncoder.addEvent({EventType::kMenuEncoderMoved, 0, {.delta=1}});
  mockEncoder.addEvent({EventType::kRawMenuEncoderPressed, 0, {}});

  pollManager.registerDevice(&mockSwitch);
  pollManager.registerDevice(&mockEncoder);

  pollManager.poll();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kRawBypassPressed, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kRawTapLongPressed, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kMenuEncoderMoved, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kRawMenuEncoderPressed, e.m_type);

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_overflow() {
  hal::PollManager pollManager;

  MockPollable mockSwitch;
  MockPollable mockEncoder;

  mockSwitch.init();
  mockEncoder.init();

  mockSwitch.addEvent({EventType::kRawBypassPressed, 0, {}});
  mockSwitch.addEvent({EventType::kRawTapLongPressed, 0, {}});
  mockSwitch.addEvent({EventType::kRawBypassPressed, 0, {}});
  mockSwitch.addEvent({EventType::kRawTapLongPressed, 0, {}});
  mockSwitch.addEvent({EventType::kRawBypassPressed, 0, {}});
  mockSwitch.addEvent({EventType::kRawTapLongPressed, 0, {}});

  mockEncoder.addEvent({EventType::kMenuEncoderMoved, 0, {.delta=1}});
  mockEncoder.addEvent({EventType::kRawMenuEncoderPressed, 0, {}});

  pollManager.registerDevice(&mockSwitch);
  pollManager.registerDevice(&mockEncoder);

  pollManager.poll();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kRawBypassPressed, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kRawTapLongPressed, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kRawBypassPressed, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kRawTapLongPressed, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kMenuEncoderMoved, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kRawMenuEncoderPressed, e.m_type);

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_register);
  RUN_TEST(test_poll);
  RUN_TEST(test_overflow);
  UNITY_END();
}