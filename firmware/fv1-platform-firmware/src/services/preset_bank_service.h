#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "periphs/eeprom.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "logic/preset.h"
#include "ui/inputs.h"

class PresetBankService : public Service {
  private:
    LogicalState& m_logicalState;
    MemoryHandler m_handler;
    EEPROM& m_eeprom;

    void syncSavePresetState();
    void loadPresetBank(uint8_t t_bankIndex, PresetBank& t_bank);
    void publishSavePresetBankEvent(const Event& t_event) const;
    void publishPresetBankValueChangedEvent(const Event& t_event) const;

  public:
    PresetBankService(LogicalState& t_lState, EEPROM& t_eeprom) :
      m_logicalState(t_lState),
      m_eeprom(t_eeprom) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;
};
