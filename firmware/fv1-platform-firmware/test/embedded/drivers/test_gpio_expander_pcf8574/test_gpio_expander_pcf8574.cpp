#include <unity.h>
#include "core/event_bus.h"
#include "drivers/gpio_driver.h"
#include "drivers/gpio_expander_pcf8574.h"
#include "drivers/gpio_expander_driver.h"
#include "logic/logical_state.h"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_p0_p1_input_gnd() {
  hal::Pcf8574Expander expander(0x00);
  expander.init();

  hal::ExpanderGpioDriver p0(0, hal::GpioConfig::kInputPullup, expander);
  hal::ExpanderGpioDriver p1(1, hal::GpioConfig::kInputPullup, expander);

  p0.init();
  p1.init();

  Event events[4];
  expander.poll(events, 4);

  TEST_ASSERT_EQUAL(0, p0.read());
  TEST_ASSERT_EQUAL(0, p1.read());
}

void setup() {
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_p0_p1_input_gnd);
  UNITY_END();
}

void loop() {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(500);
}
