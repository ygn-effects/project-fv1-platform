#include "services/preset_service.h"

void PresetService::applyPreset() {
  m_presetHandler.applyToState(m_logicalState, m_logicalState.m_currentPreset);
}

void PresetService::init() {

}

void PresetService::handleEvent(const Event& t_event) {

}

void PresetService::update() {

}

bool PresetService::interestedIn(const Event& t_event) const {

}
