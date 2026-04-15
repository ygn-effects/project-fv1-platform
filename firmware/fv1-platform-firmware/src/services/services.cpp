#include "services/services.h"

namespace services {

LogicalState logicalState;

FsmService fsmService(logicalState);
MidiService midiService(logicalState, hal::midiSerial, hal::clock);
SettingsService settingsService(logicalState, hal::eeprom, hal::clock);
PresetBankService presetBankService(logicalState, hal::eeprom);
PresetService presetService(logicalState, hal::eeprom);
ProgramModeService programModeService(logicalState, hal::programModeLed);
ProgramService programService(logicalState);
BypassService bypassService(logicalState, hal::bypass);
ExprService exprService(logicalState);
PotService potService(logicalState);
TapService tapService(logicalState);
TempoService tempoService(logicalState, hal::tapSwitchLed, hal::clock);
Fv1Service fv1Service(logicalState, hal::fv1);
CrossfadeService crossfadeService(logicalState, hal::vcaDryDac, hal::vcaWetDac);
MenuService menuService(logicalState, hal::menuLockLed, hal::clock);
DisplayService displayService(logicalState, hal::display);

ServiceManager serviceManager;

void init() {
  serviceManager.registerService(&fsmService);
  serviceManager.registerService(&midiService);
  serviceManager.registerService(&settingsService);
  serviceManager.registerService(&presetBankService);
  serviceManager.registerService(&presetService);
  serviceManager.registerService(&programModeService);
  serviceManager.registerService(&programService);
  serviceManager.registerService(&bypassService);
  serviceManager.registerService(&exprService);
  serviceManager.registerService(&potService);
  serviceManager.registerService(&tapService);
  serviceManager.registerService(&tempoService);
  serviceManager.registerService(&fv1Service);
  serviceManager.registerService(&crossfadeService);
  serviceManager.registerService(&menuService);
  serviceManager.registerService(&displayService);

  serviceManager.init();
}

void update() {
  serviceManager.handleEvents();
  serviceManager.update();
}

}
