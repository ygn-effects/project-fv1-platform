#pragma once
#include "logic/program.h"

namespace ProgramsDefinitions {
  constexpr Program kPrograms[ProgramConstants::c_maxPrograms] = {
    Program("Hall Reverb", false, false, false, true, true, true, true, "Pot 0 Label", "Pot 1 Label", "Pot 2 Label", "Mix pot Label", 0, 0),
    Program("Tape Delay", true, true, true, true, false, false, true, "Pot 0 Label", "Pot 1 Label", "Pot 2 Label", "Mix pot Label", 200, 1200),
    Program("Chorus", false, false, true, true, true, false, false, "Pot 0 Label", "Pot 1 Label", "Pot 2 Label", "Mix pot Label",0, 0),
    Program("Flanger", false, false, true, true, true, true, false, "Pot 0 Label", "Pot 1 Label", "Pot 2 Label", "Mix pot Label", 0, 0),
    Program("Plate Reverb", false, false, false, false, true, true, true, "Pot 0 Label", "Pot 1 Label", "Pot 2 Label", "Mix pot Label", 0, 0),
    Program("Analog Delay", true, true, false, true, false, false, true, "Pot 0 Label", "Pot 1 Label", "Pot 2 Label", "Mix pot Label", 150, 800),
    Program("Tremolo", false, false, true, false, false, true, true, "Pot 0 Label", "Pot 1 Label", "Pot 2 Label", "Mix pot Label", 0, 0),
    Program("Phaser", false, false, false, true, true, true, false, "Pot 0 Label", "Pot 1 Label", "Pot 2 Label", "Mix pot Label", 0, 0),
  };
}
