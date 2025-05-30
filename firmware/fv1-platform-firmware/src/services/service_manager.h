#pragma once

#include <stdint.h>
#include "core/event_bus.h"
#include "core/service.h"

namespace ServiceManagerConstants {
  static constexpr uint8_t c_maxServices = 20;
}

class ServiceManager {
  private:
    Service* m_services[ServiceManagerConstants::c_maxServices];
    uint8_t m_servicesCount;

  public:
    ServiceManager()
      : m_servicesCount(0) {}

    void registerService(Service* t_service) {
      m_services[m_servicesCount++] = t_service;
    }

    void init() {
      for (uint8_t i = 0; i < m_servicesCount; i++) {
        m_services[i]->init();
      }
    }

    void handleEvents() {
      while (EventBus::hasEvent()) {
        Event e;
        EventBus::recall(e);

        Serial.print("Received event : ");
        Serial.println(static_cast<uint8_t>(e.m_type));

        for (uint8_t i = 0; i < m_servicesCount; i++) {
          if (m_services[i]->interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type))) {
            m_services[i]->handleEvent(e);
          }
        }
      }
    }

    void update() {
      for (uint8_t i = 0; i < m_servicesCount; i++) {
        m_services[i]->update();
      }
    }
};
