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

int main() {
  UNITY_BEGIN();

  UNITY_END();
}
