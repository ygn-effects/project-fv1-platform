#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "periphs/clock.h"
#include "periphs/eeprom.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "ui/inputs.h"

namespace SettingsServiceConstants {
  static constexpr uint16_t c_editTimeout = 10000;
}

enum class DirtyBits : uint8_t {
  kBypass = 1 << 0,
  kProgramMode = 1 << 1,
  kProgram = 1 << 2,
  kTap = 1 << 3,
  kTempo = 1 << 4,
  kExpr = 1 << 5,
  kPot = 1 << 6
};

enum class DirtyPotBits : uint8_t {
  kPot0 = 1 << 0,
  kPot1 = 1 << 1,
  kPot2 = 1 << 2,
  kMixPot = 1 << 3
};

class SettingsService : public Service {
  private:
    LogicalState& m_logicalState;
    MemoryHandler m_handler;
    EEPROM& m_eeprom;
    Clock& m_clock;

    uint32_t m_lastEditTime{0};
    uint8_t m_dirtyFlags{0};
    uint8_t m_dirtyPotFlags{0};

    void saveRegion(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_potIndex = 0);
    void loadRegion(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_potIndex = 0);

  public:
    SettingsService(LogicalState& t_lState, EEPROM& t_eeprom, Clock& t_clock) :
      m_logicalState(t_lState),
      m_eeprom(t_eeprom),
      m_clock(t_clock) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;
};
