#include "services/fsm_service.h"

void FsmService::transitionTo(AppState t_state) {
  m_state = t_state;
}

bool FsmService::isPressed(const Event& t_event, const SwitchId t_id) const {
  return (t_event.m_subject == EventSubject::kSwitch
          && t_event.m_action == EventAction::kPressed
          && t_event.matchesId(static_cast<uint8_t>(t_id)));
}

bool FsmService::isLongPressed(const Event& t_event, const SwitchId t_id) const {
  return (t_event.m_subject == EventSubject::kSwitch
          && t_event.m_action == EventAction::kLongPressed
          && t_event.matchesId(static_cast<uint8_t>(t_id)));
}

bool FsmService::isDeltaChanged(const Event& t_event) const {
  return (t_event.m_subject == EventSubject::kEncoder
          && t_event.m_action == EventAction::kDeltaChanged);
}

bool FsmService::isValueChanged(const Event& t_event) const {
  return (t_event.m_subject == EventSubject::kPot
          && t_event.m_action == EventAction::kValueChanged);
}

bool FsmService::isBypassToggled(const Event& t_event) const {
  return (t_event.m_domain == EventDomain::kLogic
          && t_event.m_subject == EventSubject::kBypass
          && t_event.m_action == EventAction::kToggled);
}

bool FsmService::isMenuUnlocked(const Event& t_event) const {
  return (t_event.m_domain == EventDomain::kUI
          && t_event.m_subject == EventSubject::kMenu
          && t_event.m_action == EventAction::kUnlocked);
}

bool FsmService::isMenuLocked(const Event& t_event) const {
  return (t_event.m_domain == EventDomain::kUI
          && t_event.m_subject == EventSubject::kMenu
          && t_event.m_action == EventAction::kLocked);
}

bool FsmService::isProgramModeToggled(const Event& t_event) const {
  return (t_event.m_subject == EventSubject::kProgramMode
          && t_event.m_action == EventAction::kToggled);
}

void FsmService::rePublishPhysicalEvent(const Event& t_event) const {
  Event e = t_event;
  e.m_domain = EventDomain::kPhysical;
  EventBus::publish(e);
}

void FsmService::init() {
  transitionTo(AppState::kRestoreState);
}

void FsmService::handleEvent(const Event& t_event) {
  switch(m_state) {
    case AppState::kBoot:
    case AppState::kRestoreState:
      if (t_event.m_action == EventAction::kBooted) {
        if (m_logicalState.m_bypassState == BypassState::kActive) {
          transitionTo(m_logicalState.m_programMode == ProgramMode::kProgram
                        ? AppState::kProgramIdle
                        : AppState::kPresetIdle);
        }
        else {
          transitionTo(AppState::kBypassed);
        }
      }

      break;

    case AppState::kBypassed:
      // Bypass Press
      if (isPressed(t_event, SwitchId::kBypass)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Bypass toggle logic event
      if (isBypassToggled(t_event)) {
        transitionTo(m_logicalState.m_programMode == ProgramMode::kProgram
                      ? AppState::kProgramIdle
                      : AppState::kPresetIdle);
      }

      break;

    case AppState::kProgramIdle:
      // Bypass Press
      if (isPressed(t_event, SwitchId::kBypass)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Bypass state toggled
      if (isBypassToggled(t_event)) {
        transitionTo(AppState::kBypassed);
        return;
      }

      // Tap
      if (isPressed(t_event, SwitchId::kTap)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Long‑tap
      if (isLongPressed(t_event, SwitchId::kTap)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Encoder‑switch long press
      if (isLongPressed(t_event, SwitchId::kMenuEncoder)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Menu unlocked
      if (isMenuUnlocked(t_event)) {
        transitionTo(AppState::kProgramEdit);
        return;
      }

      // Program mode switch long press
      if (isLongPressed(t_event, SwitchId::kProgramMode)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Program mode toggle
      if (isProgramModeToggled(t_event)) {
        transitionTo(AppState::kPresetIdle);
        return;
      }

      break;

    case AppState::kProgramEdit:
      // Bypass Press
      if (isPressed(t_event, SwitchId::kBypass)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Menu unlocked
      if (isMenuLocked(t_event)) {
        transitionTo(AppState::kProgramIdle);
        return;
      }

      // Menu encoder press
      if (isPressed(t_event, SwitchId::kMenuEncoder)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Encoder‑switch long press
      if (isLongPressed(t_event, SwitchId::kMenuEncoder)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Menu encoder moved
      if (isDeltaChanged(t_event)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Tap
      if (isPressed(t_event, SwitchId::kTap)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Long‑tap
      if (isLongPressed(t_event, SwitchId::kTap)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Pot move
      if (isValueChanged(t_event)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Bypass state toggled
      if (isBypassToggled(t_event)) {
        transitionTo(AppState::kBypassed);
        return;
      }

      break;

    case AppState::kPresetIdle:
      // Bypass Press
      if (isPressed(t_event, SwitchId::kBypass)) {
        rePublishPhysicalEvent(t_event);
        return;
      }
      // Program mode switch long press
      if (isLongPressed(t_event, SwitchId::kProgramMode)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Bypass state toggled
      if (isBypassToggled(t_event)) {
        transitionTo(AppState::kBypassed);
        return;
      }

      // Program mode toggle
      if (isProgramModeToggled(t_event)) {
        transitionTo(AppState::kProgramIdle);
        return;
      }

      // Tap
      if (isPressed(t_event, SwitchId::kTap)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      // Long‑tap
      if (isLongPressed(t_event, SwitchId::kTap)) {
        rePublishPhysicalEvent(t_event);
        return;
      }

      break;

    default:
      break;
  }
}

void FsmService::update() {

}

bool FsmService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kSystem) {
    if (t_event.m_action == EventAction::kBooted) return true;
  }

  if (t_event.m_domain == EventDomain::kDriver) return true;

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_action == EventAction::kLocked
        || t_event.m_action == EventAction::kUnlocked) return true;
  }

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kBypass
        && t_event.m_action == EventAction::kToggled) return true;

    if (t_event.m_subject == EventSubject::kProgramMode
        && t_event.m_action == EventAction::kToggled) return true;
  }

  return false;
}

AppState FsmService::getAppState() const {
  return m_state;
}

void FsmService::setAppState(const AppState t_state) {
  m_state = t_state;
}