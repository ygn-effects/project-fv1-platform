#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/menu_service.h"

#include "../src/services/menu_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

int main() {
  UNITY_BEGIN();

  UNITY_END();
}