#include "services/menu_service.h"

MenuService::MenuService(LogicalState& t_lState, Clock& t_clock)
    : m_logicState(t_lState), m_clock(t_clock) {}

void MenuService::syncSavePresetState() {
  m_logicState.m_saveTargetBank = m_logicState.m_currentPresetBank;
  m_logicState.m_saveTargetPreset = m_logicState.m_currentPreset;
}

void MenuService::publishUIMenuLockedEvent() const {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kLocked;

  EventBus::publish(e);
}

void MenuService::publishUIMenuUnlockedEvent() const {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kUnlocked;

  EventBus::publish(e);
}

void MenuService::publishUIMenuUpdatedEvent() {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kUpdated;
  e.m_data.ptr = static_cast<void*>(&m_handler.m_view);

  EventBus::publish(e);
}

void MenuService::publishViewUpdate() {
  m_handler.buildView(m_logicState);
  publishUIMenuUpdatedEvent();
}

void MenuService::handleLocked(const Event& t_event) {
  if (t_event.m_action == EventAction::kLongPressed
      && t_event.matchesId(SwitchId::kMenuLock)) {
    m_handler.unlock(m_logicState);
    m_lastInputTime = t_event.m_timestamp;

    publishUIMenuUnlockedEvent();
    publishViewUpdate();
  }

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) {
      publishViewUpdate();
    }

    if (t_event.m_subject == EventSubject::kTempo
        && t_event.m_action == EventAction::kValueChanged) {
      publishViewUpdate();
    }
  }
}

void MenuService::handleUnlocked(const Event& t_event) {
  if (m_handler.m_subState == SubState::kSelecting) {
    if (t_event.m_domain == EventDomain::kPhysical) {
      if (t_event.m_subject == EventSubject::kEncoder
          && t_event.matchesId(EncoderId::kMenuEncoder)) {
        handleSelecting(t_event);
        publishViewUpdate();
        return;
      }

      if (t_event.m_subject == EventSubject::kSwitch
          && t_event.m_action == EventAction::kPressed
          && t_event.matchesId(SwitchId::kMenuEncoder)) {
        handleSelecting(t_event);
        publishViewUpdate();
        return;
      }

      if (t_event.m_subject == EventSubject::kSwitch
          && t_event.m_action == EventAction::kLongPressed
          && t_event.matchesId(SwitchId::kMenuLock)) {
        m_handler.lock();
        publishUIMenuLockedEvent();
        publishViewUpdate();
        return;
      }

      if (t_event.m_subject == EventSubject::kSwitch
          && t_event.m_action == EventAction::kLongPressed
          && t_event.matchesId(static_cast<uint8_t>(SwitchId::kMenuEncoder))) {
        handlePresetSaving(t_event);
        publishViewUpdate();
      }
    }

    if (t_event.m_domain == EventDomain::kLogic) {
      if (t_event.m_subject == EventSubject::kTempo
          && t_event.m_action == EventAction::kValueChanged) {
        handleTempoChange(t_event);
        publishViewUpdate();
        return;
      }

      if (t_event.m_subject == EventSubject::kBypass
          && t_event.m_action == EventAction::kToggled) {
        m_handler.lock();
        publishUIMenuLockedEvent();
        publishViewUpdate();
        return;
      }

      if (t_event.m_subject == EventSubject::kPot
          && t_event.m_action == EventAction::kValueChanged) {
        handlePotsMoving(t_event);
        publishViewUpdate();
        return;
      }
    }

    if (t_event.m_domain == EventDomain::kUI) {
      if (t_event.m_subject == EventSubject::kPreset
          && t_event.m_action == EventAction::kSettingChanged) {
        switch (static_cast<SavePresetParam>(t_event.m_id)) {
          case SavePresetParam::kTargetBank:
            m_logicState.m_saveTargetBank = Utils::clampedAdd(m_logicState.m_saveTargetBank, t_event.m_data.delta, PresetConstants::c_presetBankCount - 1);
            break;

          case SavePresetParam::kTargetPreset:
            m_logicState.m_saveTargetPreset = Utils::clampedAdd(m_logicState.m_saveTargetPreset, t_event.m_data.delta, PresetConstants::c_presetPerBank - 1);
            break;

          default:
            break;
        }

        publishViewUpdate();
        return;
      }
    }
  }
  else {
    if (t_event.m_subject == EventSubject::kEncoder
        && t_event.matchesId(EncoderId::kMenuEncoder)) {
      handleEditing(t_event);
      publishViewUpdate();
    }
    else if (t_event.m_subject == EventSubject::kSwitch
        && t_event.m_action == EventAction::kPressed
        && t_event.matchesId(SwitchId::kMenuEncoder)) {
      handleEditing(t_event);
      publishViewUpdate();
    }
  }
}

void MenuService::handleSelecting(const Event& t_event) {
  switch (t_event.m_action) {
    case EventAction::kDeltaChanged:
      m_handler.moveCursor(t_event.m_data.delta, m_logicState);
      break;

    case EventAction::kPressed:
      m_handler.handleSelect(m_logicState);
      break;

    default:
      break;
  }

  m_lastInputTime = t_event.m_timestamp;
}

void MenuService::handleEditing(const Event& t_event) {
  switch (t_event.m_action) {
    case EventAction::kDeltaChanged:
      m_handler.applyEditDelta(t_event.m_data.delta, m_logicState);
      break;

    case EventAction::kPressed:
      m_handler.endEditing();
      break;

    default:
      break;
  }

  m_lastInputTime = t_event.m_timestamp;
}

void MenuService::handlePotsMoving(const Event& t_event) {
  if (m_tempoMenuActive || m_savePresetMenuActive) {
    m_tempoMenuActive = false;
    m_savePresetMenuActive = false;
    m_handler.popOverlay();
  }

  if (m_potMenuActive) {
    m_handler.popOverlay();
  }

  m_handler.pushPotOverlay(t_event.m_id, m_logicState);
  m_potMenuActive = true;
  m_lastPotMoveTime = t_event.m_timestamp;
  m_lastInputTime = t_event.m_timestamp;
}

void MenuService::handleTempoChange(const Event& t_event) {
  if (m_potMenuActive || m_savePresetMenuActive) {
    m_potMenuActive = false;
    m_savePresetMenuActive = false;
    m_handler.popOverlay();
  }

  if (!m_tempoMenuActive) {
    m_handler.pushTempoOverlay();
  }

  m_tempoMenuActive = true;
  m_lastTempoChangeTime = t_event.m_timestamp;
  m_lastInputTime = t_event.m_timestamp;
}

void MenuService::handlePresetSaving(const Event& t_event) {
  if (m_potMenuActive || m_tempoMenuActive) {
    m_potMenuActive = false;
    m_tempoMenuActive = false;
    m_handler.popOverlay();
  }

  if (!m_savePresetMenuActive) {
    m_handler.pushPresetSaveOverlay();
  }

  m_savePresetMenuActive = true;
  m_lastInputTime = t_event.m_timestamp;
}

void MenuService::init() {
  m_handler.init();

  if (m_logicState.m_programMode == ProgramMode::kProgram) {
    m_handler.unlock(m_logicState);
  }

  syncSavePresetState();
  publishViewUpdate();
}

void MenuService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) {
      publishViewUpdate();

      if (m_logicState.m_programMode == ProgramMode::kPreset) {
        syncSavePresetState();
      }
    }
  }

  switch (m_handler.m_mode) {
    case UiMode::kLocked:
      handleLocked(t_event);
      return;

    case UiMode::kUnlocked:
      handleUnlocked(t_event);
      return;

    default:
      break;
  }
}

void MenuService::update() {
  if (m_handler.m_mode == UiMode::kLocked) return;

  uint32_t now = m_clock.now();

  if (m_logicState.m_programMode == ProgramMode::kPreset) {
    if ((now - m_lastInputTime) > ui::MenuConstants::c_menuTimeout) {
      m_handler.lock();
      publishUIMenuLockedEvent();
      publishViewUpdate();
    }
  }

  if (m_potMenuActive) {
    if ((now - m_lastPotMoveTime) > ui::MenuConstants::c_potMenuTimeout) {
      m_potMenuActive = false;
      m_handler.popOverlay();
      publishViewUpdate();
    }
  }

  if (m_tempoMenuActive) {
    if ((now - m_lastTempoChangeTime) > ui::MenuConstants::c_tempoMenuTimeout) {
      m_tempoMenuActive = false;
      m_handler.popOverlay();
      publishViewUpdate();
    }
  }
}

bool MenuService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kSwitch
        && t_event.m_action == EventAction::kLongPressed
        && t_event.matchesId(SwitchId::kMenuLock)) return true;

    if (t_event.m_subject == EventSubject::kEncoder
        && t_event.matchesId(EncoderId::kMenuEncoder)) return true;

    if (t_event.m_subject == EventSubject::kSwitch
        && t_event.matchesId(SwitchId::kMenuEncoder)) return true;
  }

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kTempo
        && t_event.m_action == EventAction::kValueChanged) return true;

    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) return true;

    if (t_event.m_subject == EventSubject::kBypass
        && t_event.m_action == EventAction::kToggled) return true;

    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kSettingChanged) return true;
  }

  return false;
}
