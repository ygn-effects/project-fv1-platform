#pragma once

#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/fsm.h"
#include "logic/memory_handler.h"
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
#include "mock/mock_eeprom.h"
#include "mock/mock_fv1.h"
#include "mock/mock_bypass.h"
#include "mock/mock_clock.h"
#include "mock/mock_dac.h"
#include "mock/mock_adjustable.h"
#include "mock/mock_led.h"
#include "mock/mock_display.h"
#include "mock/mock_serial.h"


class InteractionFixture {
  public:
    LogicalState logicalState;

    MockEEPROM mockEeprom;
    MockFv1 mockFv1;
    MockBypass mockBypass;
    MockedClock mockClock;
    MockAdjustable mockTapLed;
    MockDisplay mockDisplay;
    MockedSerial mockSerial;
    MockDac mockDacDry;
    MockDac mockDacWet;
    MockLed mockProgramModeLed;
    MockLed mockMenuLockLed;

    FsmService fsmService;
    MidiService midiService;
    SettingsService settingsService;
    PresetBankService presetBankService;
    PresetService presetService;
    ProgramModeService programModeService;
    ProgramService programService;
    BypassService bypassService;
    ExprService exprService;
    PotService potService;
    TapService tapService;
    TempoService tempoService;
    Fv1Service fv1Service;
    CrossfadeService crossfadeService;

    MenuService menuService;
    DisplayService displayService;

    ServiceManager serviceManager;

    InteractionFixture()
      : fsmService(logicalState)
      , midiService(logicalState, mockSerial, mockClock)
      , settingsService(logicalState, mockEeprom)
      , presetBankService(logicalState, mockEeprom)
      , presetService(logicalState, mockEeprom)
      , programModeService(logicalState, mockProgramModeLed)
      , programService(logicalState)
      , bypassService(logicalState, mockBypass)
      , exprService(logicalState)
      , potService(logicalState)
      , tapService(logicalState)
      , tempoService(logicalState, mockTapLed, mockClock)
      , fv1Service(logicalState, mockFv1)
      , crossfadeService(logicalState, mockDacDry, mockDacWet)
      , menuService(logicalState, mockMenuLockLed, mockClock)
      , displayService(logicalState, mockDisplay)
    {
      clearEventBus();
      registerServices();
    }

    void init() {
      serviceManager.init();
    }

    void dispatchAllEvents() {
      serviceManager.handleEvents();
    }

    void dispatchEvent() {
      serviceManager.handleEvent();
    }

    void publish(const Event& t_event) {
      EventBus::publish(t_event);
    }

    void publishAndDispatchEvent(const Event& t_event) {
      EventBus::publish(t_event);
      dispatchEvent();
    }

    void publishAndDispatchAllEvents(const Event& t_event) {
      EventBus::publish(t_event);
      dispatchAllEvents();
    }

    void clearEventBus() {
      Event e;
      while (EventBus::hasEvent()) {
        EventBus::recall(e);
      }
    }

    bool findEvent(EventDomain t_domain, EventSubject t_subject, EventAction t_action) {
      Event e;
      while (EventBus::hasEvent()) {
        EventBus::recall(e);
        if (e.m_domain == t_domain && e.m_subject == t_subject && e.m_action == t_action) {
          return true;
        }
      }

      return false;
    }

    bool recallEvent(Event& t_event) {
      if (EventBus::hasEvent()) {
        EventBus::recall(t_event);
        return true;
      }

      return false;
    }

    bool hasEvents() const {
      return EventBus::hasEvent();
    }

    void updateAllServices() {
      serviceManager.update();
    }

    void syncEepromWithState() {
      MemoryHandler handler;
      uint8_t buffer[512];
      handler.serializeRegion(MemoryRegion::kLogicalState, logicalState, buffer);
      RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kLogicalState);
      mockEeprom.write(info.m_address, buffer, info.m_length);
    }

    void SyncEepromWithLoadedPresetBank() {
      for (uint8_t i = 0; i < PresetConstants::c_presetPerBank; i++) {
        MemoryHandler handler;
        uint8_t buffer[512];
        RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kPreset, logicalState.m_currentPresetBank, i);
        handler.serializePreset(logicalState.m_loadedPresetBank.m_presets[i], buffer, logicalState.m_currentPresetBank, i, 0);
        mockEeprom.write(info.m_address, buffer, info.m_length);
      }
    }

    void resetMocks() {
      mockEeprom.reset();
      mockFv1.m_potValues.clear();
      mockFv1.m_s0 = 0;
      mockFv1.m_s1 = 0;
      mockFv1.m_s2 = 0;
      mockBypass.m_kState = 0;
      mockBypass.m_okState = 0;
      mockClock.setClock(0);
      mockTapLed.reset();
      mockDisplay.reset();
    }

    void advanceTime(uint32_t t_ms) {
      mockClock.advanceBy(t_ms);
    }

    void setTime(uint32_t t_ms) {
      mockClock.setClock(t_ms);
    }

  private:
    void registerServices() {
      serviceManager.registerService(&fsmService);
      serviceManager.registerService(&midiService);
      serviceManager.registerService(&settingsService);
      serviceManager.registerService(&programModeService);
      serviceManager.registerService(&presetBankService);
      serviceManager.registerService(&presetService);
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
    }
};
