#include "services/crossfade_service.h"

void CrossfadeService::syncHandler() {
  m_handler.m_currentCurve = m_logicalState.m_activeProgram->m_crossfadeParams.m_curve;
  m_handler.m_minInputValue = m_logicalState.m_activeProgram->m_crossfadeParams.m_minValue;
  m_handler.m_maxInputValue = m_logicalState.m_activeProgram->m_crossfadeParams.m_maxValue;
}

void CrossfadeService::applyMix(uint16_t t_mixValue) {
  CrossfadeResult result = m_handler.calculate(t_mixValue);

  m_dacDry.write(result.m_dry);
  m_dacWet.write(result.m_wet);
}

void CrossfadeService::init() {
  m_dacDry.init();
  m_dacWet.init();

  syncHandler();
}

void CrossfadeService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kLogic) {
    if(t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) {
      syncHandler();
      applyMix(m_logicalState.m_potParams[m_logicalState.m_currentProgram][static_cast<uint8_t>(PotId::kMixPot)].m_value);

      return;
    }

    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) {
      applyMix(t_event.m_data.value);
    }
  }
}

void CrossfadeService::update() {

}

bool CrossfadeService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) return true;

    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged
        && t_event.matchesId(PotId::kMixPot)) return true;
  }

  return false;
}
