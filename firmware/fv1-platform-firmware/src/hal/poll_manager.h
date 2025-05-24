#pragma once

#include <stdint.h>
#include "core/event.h"
#include "core/event_bus.h"
#include "periphs/pollable.h"
#include "utils/circular_buffer.h"

namespace hal {

namespace PollManagerConstants {
  static constexpr uint8_t c_maxPollDevices = 16;
  static constexpr uint8_t c_maxEventsPerDevice = 4;
  static constexpr uint8_t c_maxEvents = 16;
}

class PollManager {
  private:
    Pollable* m_devices[PollManagerConstants::c_maxPollDevices];
    Event m_buffer[PollManagerConstants::c_maxEventsPerDevice];
    uint8_t m_devicesCount;

  public:
    PollManager()
      : m_devicesCount(0) {}

    void registerDevice(Pollable* t_device) {
      m_devices[m_devicesCount++] = t_device;
    }

    void init() {
      for (auto* device : m_devices) {
        device->init();
      }
    }

    void poll() {
      for (auto* device : m_devices) {
        size_t eventCount = device->poll(m_buffer, PollManagerConstants::c_maxEventsPerDevice);

        for (uint8_t i = 0; i < eventCount; i++) {
          EventBus::publish(m_buffer[i]);
        }
      }
    }

    // Debug
    uint8_t getDevicesCount() { return m_devicesCount; }
};

}
