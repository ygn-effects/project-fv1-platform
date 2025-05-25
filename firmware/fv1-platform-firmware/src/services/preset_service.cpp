#include "services/preset_service.h"

void PresetService::applyPreset() {
  m_handler.applyToState(m_logicalState, m_logicalState.m_currentPreset);
}

void PresetService::publishLoadBankEvent(const Event& t_event) {
  Event e;
  e.m_type = EventType::kLoadPresetBank;
  e.m_timestamp = t_event.m_timestamp; /*millis*/
  e.m_data.value = m_logicalState.m_currentPresetBank;
  EventBus::publish(e);
}

void PresetService::publishSaveCurrentPresetBank(const Event& t_event) {
  EventBus::publish({EventType::kSaveCurrentPresetBank, t_event.m_timestamp /*millis()*/, {}});
}

void PresetService::publishSaveCurrentPreset(const Event& t_event) {
  EventBus::publish({EventType::kSaveCurrentPreset, t_event.m_timestamp /*millis()*/, {}});
}

void PresetService::init() {

}

PresetService::PresetService(LogicalState& t_lState)
  : m_logicalState(t_lState) {}

void PresetService::handleEvent(const Event& t_event) {
  switch (t_event.m_type) {
    case EventType::kPresetChanged:
    case EventType::kMenuPresetChanged:
      m_logicalState.m_currentPreset = Utils::wrappedAdd(m_logicalState.m_currentPreset, t_event.m_data.delta, PresetConstants::c_presetPerBank);
      applyPreset();
      publishSaveCurrentPreset(t_event);
      break;

    case EventType::kPresetBankChanged:
    case EventType::kMenuPresetBankChanged:
      m_logicalState.m_currentPresetBank = Utils::wrappedAdd(m_logicalState.m_currentPresetBank, t_event.m_data.delta, PresetConstants::c_presetBankCount);
      m_logicalState.m_currentPreset = 0;
      publishLoadBankEvent(t_event);
      publishSaveCurrentPresetBank(t_event);
      publishSaveCurrentPreset(t_event);
      break;

    case EventType::kMenuSavePreset:
      m_handler.snapshotFromState(m_logicalState);
      Event e;
      e.m_type = EventType::kSavePreset;
      e.m_timestamp = t_event.m_timestamp; /*millis*/
      e.m_data.value = t_event.m_data.value;
      EventBus::publish(e);
      break;

    case EventType::kPresetBankLoaded:
      m_handler.m_currentPresetBank = t_event.m_data.bank;
      if (m_logicalState.m_programMode == ProgramMode::kPreset) { applyPreset(); }
      break;

    case EventType::kProgramModeChanged:
      if (m_logicalState.m_programMode == ProgramMode::kPreset) { applyPreset(); }
      break;

    default:
      break;
  }
}

void PresetService::update() {

}

bool PresetService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kPresetChangedEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kPresetBankChangedEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramModeChangedEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPresetChangedEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPresetBankChangedEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPresetSaveEvent)
      || (t_category == EventCategory::kLoadEvent && t_subCategory == EventSubCategory::kBankLoadEvent);
}
