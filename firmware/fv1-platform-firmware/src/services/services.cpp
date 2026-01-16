#include "services/services.h"

namespace services {

LogicalState logicalState;

FsmService fsmService(logicalState);
SettingsService settingsService(logicalState, hal::eeprom);
ProgramModeService programModeService(logicalState);
ProgramService programService(logicalState);
BypassService bypassService(logicalState, hal::bypass);
PotService potService(logicalState);
TapService tapService(logicalState);
TempoService tempoService(logicalState, hal::tapSwitchLed, hal::clock);
MenuService menuService(logicalState, hal::clock);
DisplayService displayService(logicalState, hal::display);

ServiceManager serviceManager;

void init() {
  serviceManager.registerService(&fsmService);
  serviceManager.registerService(&settingsService);
  serviceManager.registerService(&programModeService);
  serviceManager.registerService(&programService);
  serviceManager.registerService(&bypassService);
  serviceManager.registerService(&potService);
  serviceManager.registerService(&tapService);
  serviceManager.registerService(&tempoService);
  serviceManager.registerService(&menuService);
  serviceManager.registerService(&displayService);

  serviceManager.init();
}

void update() {
  serviceManager.handleEvents();
  serviceManager.update();
}

}