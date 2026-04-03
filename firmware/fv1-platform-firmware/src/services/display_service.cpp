#include "services/display_service.h"

void DisplayService::init() {
  m_display.init();
  m_renderer.init();
}

void DisplayService::handleEvent(const Event& t_event) {
  const auto* view = static_cast<const ui::MenuView*>(t_event.m_data.ptr);
  m_display.clear();
  m_renderer.drawMenu(*view, m_logicalState);
  m_display.display();
}

void DisplayService::update() {

}

bool DisplayService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kUI
      && t_event.m_subject == EventSubject::kMenu
      && t_event.m_action == EventAction::kUpdated) return true;

  return false;
}
