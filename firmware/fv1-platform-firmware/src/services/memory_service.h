#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "hal/eeprom.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "logic/preset_handler.h"

class MemoryService : public Service {
  private:
    LogicalState& m_logicalState;
    MemoryHandler m_handler;
    EEPROM& m_eeprom;

    PresetBank m_loadedBank;

    void saveRegion(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_potIndex = 0);
    void loadRegion(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_potIndex = 0);

    void loadPresetBank(uint8_t t_bankIndex);
    void savePreset(uint8_t t_bankIndex, uint8_t t_presetIndex);

  public:
    MemoryService(LogicalState& t_lState, EEPROM& t_eeprom);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;

    // Debug
    PresetBank& getLoadedBank() { return m_loadedBank; }
};