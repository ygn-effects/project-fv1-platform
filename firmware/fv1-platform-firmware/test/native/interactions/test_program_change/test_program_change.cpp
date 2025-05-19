#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/bypass_service.h"
#include "services/expr_service.h"
#include "services/fsm_service.h"
#include "services/menu_service.h"
#include "services/midi_service.h"
#include "services/pot_service.h"
#include "services/preset_service.h"
#include "services/program_service.h"
#include "services/tap_service.h"
#include "services/tempo_service.h"

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
#include "../src/services/menu_service.cpp"
#include "../src/services/midi_service.cpp"
#include "../src/services/pot_service.cpp"
#include "../src/services/preset_service.cpp"
#include "../src/services/program_service.cpp"
#include "../src/services/tap_service.cpp"
#include "../src/services/tempo_service.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_program_change() {
  LogicalState logicalState;

  FsmService fsmService(logicalState);
  ProgramService programService(logicalState);
  PotService potService(logicalState);
  ExprService exprService(logicalState);
  TapService tapService(logicalState);
  TempoService tempoService(logicalState);
  MenuService menuService(logicalState);

  Service* services[] = {
    &fsmService,
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
}

int main() {
  UNITY_BEGIN();

  UNITY_END();
}
