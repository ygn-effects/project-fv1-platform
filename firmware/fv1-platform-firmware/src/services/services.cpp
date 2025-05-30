#include "services/services.h"

namespace services {

LogicalState logicalState;

PollService pollService(logicalState);
FsmService fsmService(logicalState);
MemoryService memoryService(logicalState, hal::eeprom);
ProgramModeService programModeService(logicalState);
ProgramService programService(logicalState);
BypassService bypassService(logicalState, hal::bypass);

ServiceManager serviceManager;

void init() {
  serviceManager.registerService(&pollService);
  serviceManager.registerService(&fsmService);
  serviceManager.registerService(&memoryService);
  serviceManager.registerService(&programModeService);
  serviceManager.registerService(&programService);
  serviceManager.registerService(&bypassService);

  serviceManager.init();
}

void update() {
  serviceManager.handleEvents();
  serviceManager.update();
}

}