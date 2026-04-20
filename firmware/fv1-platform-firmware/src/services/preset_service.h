#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "logic/preset_handler.h"
#include "periphs/eeprom.h"
#include "ui/inputs.h"
#include "ui/settings.h"
#include "utils/utils.h"

class PresetService : public Service {
  private:
    LogicalState& m_logicalState;
    PresetHandler m_presetHandler;
    MemoryHandler m_memoryHandler;
    EEPROM& m_eeprom;

    void syncSavePresetState();
    void applyPreset();
    void savePreset(uint8_t t_bankIndex, uint8_t t_presetIndex);
    void publishPresetValueChangedEvent(const Event& t_event);
    void publishSavePresetEvent(const Event& t_event);

  public:
    PresetService(LogicalState& t_lState, EEPROM& t_eeprom) :
      m_logicalState(t_lState),
      m_eeprom(t_eeprom) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;
};
