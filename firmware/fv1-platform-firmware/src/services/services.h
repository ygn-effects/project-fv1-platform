#pragma once

#include "hal/hal.h"
#include "services/service_manager.h"
#include "services/fsm_service.h"
#include "services/midi_service.h"
#include "services/settings_service.h"
#include "services/preset_bank_service.h"
#include "services/preset_service.h"
#include "services/program_mode_service.h"
#include "services/program_service.h"
#include "services/bypass_service.h"
#include "services/expr_service.h"
#include "services/pot_service.h"
#include "services/tap_service.h"
#include "services/tempo_service.h"
#include "services/fv1_service.h"
#include "services/crossfade_service.h"
#include "services/menu_service.h"
#include "services/display_service.h"
#include "services/service_manager.h"

namespace services {

void init();
void update();

}
