#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "periphs/eeprom.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"

class SettingsService : public Service {
  private:
    LogicalState& m_logicalState;
    MemoryHandler m_handler;
    EEPROM& m_eeprom;

    void saveRegion(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_potIndex = 0);
    void loadRegion(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_potIndex = 0);

    void loadPresetBank(uint8_t t_bankIndex);
    void savePreset(uint8_t t_bankIndex, uint8_t t_presetIndex);

  public:
    SettingsService(LogicalState& t_lState, EEPROM& t_eeprom);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;
};