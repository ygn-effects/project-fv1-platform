#include "services/display_service.h"

DisplayService::DisplayService(const LogicalState& t_state, Display& t_display)
  : m_logicalState(t_state),
    m_display(t_display),
    m_renderer(t_display) {}

void DisplayService::init() {
  m_display.init();
}

void DisplayService::handleEvent(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuViewUpdated) {
    const auto& view = *t_event.m_data.view;
    m_renderer.drawMenu(view, m_logicalState);
    m_display.clear();
    m_display.display();
  }
}

void DisplayService::update() {

}

bool DisplayService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuViewEvent;
}
