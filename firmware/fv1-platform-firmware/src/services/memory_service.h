#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "logic/preset_handler.h"
#include "mock/mock_eeprom.h"

class MemoryService : public Service {
  private:
    LogicalState& m_logicalState;
    PresetBank& m_presetBank;
    MemoryHandler m_handler;
    MockEEPROM* m_eeprom;

    void saveRegion(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_potIndex = 0);
    void loadRegion(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_potIndex = 0);

    void loadPresetBank(uint8_t t_bankIndex);
    void savePreset(uint8_t t_bankIndex, uint8_t t_presetIndex);

  public:
    MemoryService(LogicalState& t_lState, PresetBank& t_presetBank);
    MemoryService(LogicalState& t_lState, PresetBank& t_presetBank, MockEEPROM* t_eeprom);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};