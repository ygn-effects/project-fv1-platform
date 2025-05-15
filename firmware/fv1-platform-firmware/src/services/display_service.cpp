#include "services/display_service.h"

DisplayService::DisplayService(const LogicalState& t_state, DisplayDriver& t_driver)
  : m_logicalState(t_state),
    m_driver(t_driver),
    m_renderer(t_driver) {}

void DisplayService::init() {
  m_driver.init();
}

void DisplayService::handleEvent(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuViewUpdated) {
    const auto& view = *t_event.m_data.view;
    m_renderer.drawMenu(view, m_logicalState);
    m_driver.clear();
    m_driver.display();
  }
}

void DisplayService::update() {

}

bool DisplayService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuViewEvent;
}
