#include "services/program_mode_service.h"
#include "ui/inputs.h"

void ProgramModeService::publishSaveLogicalStateEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kSave;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void ProgramModeService::publishLoadLogicalStateEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kLoad;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void ProgramModeService::publishSaveProgramModeEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kSave;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void ProgramModeService::publishProgramModeToggledEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void ProgramModeService::init() {
  m_programModeLed.init();

  if (m_logicalState.m_bypassState == BypassState::kActive) m_programModeLed.on();
}

void ProgramModeService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kSwitch
        && t_event.m_action == EventAction::kLongPressed
        && t_event.matchesId(SwitchId::kProgramMode)) {
      m_logicalState.m_programMode == ProgramMode::kProgram
        ? publishSaveLogicalStateEvent(t_event)
        : publishLoadLogicalStateEvent(t_event);

      publishProgramModeToggledEvent(t_event);
      publishSaveProgramModeEvent(t_event);
      return;
    }
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kProgramMode
        && t_event.m_action == EventAction::kValueChanged) {
      if (m_logicalState.m_programMode == ProgramMode::kProgram
          && t_event.m_data.value == MidiCCValues::c_presetMode) {
        publishSaveLogicalStateEvent(t_event);

        publishProgramModeToggledEvent(t_event);
        publishSaveProgramModeEvent(t_event);
        return;
      }

      if (m_logicalState.m_programMode == ProgramMode::kPreset
          && t_event.m_data.value == MidiCCValues::c_programMode) {
        publishLoadLogicalStateEvent(t_event);

        publishProgramModeToggledEvent(t_event);
        publishSaveProgramModeEvent(t_event);
        return;
      }
    }
  }

  // Set logical state after it has been restored to avoid overwriting
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgramMode
        && t_event.m_action == EventAction::kToggled) {
      m_logicalState.m_programMode == ProgramMode::kProgram
        ? m_logicalState.m_programMode = ProgramMode::kPreset
        : m_logicalState.m_programMode = ProgramMode::kProgram;

      return;
    }

    if (t_event.m_subject == EventSubject::kBypass
        && t_event.m_action == EventAction::kToggled) {
      m_logicalState.m_bypassState == BypassState::kActive
        ? m_programModeLed.on()
        : m_programModeLed.off();
    }
  }
}

void ProgramModeService::update() {

}

bool ProgramModeService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kSwitch
        && t_event.m_action == EventAction::kLongPressed
        && t_event.matchesId(SwitchId::kProgramMode)) return true;
  }

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgramMode
        && t_event.m_action == EventAction::kToggled) return true;

    if (t_event.m_subject == EventSubject::kBypass
        && t_event.m_action == EventAction::kToggled) return true;
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kProgramMode
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  return false;
}
