#include "services/poll_service.h"

PollService::PollService(const LogicalState& t_lState)
  : m_logicalState(t_lState) {}

void PollService::init() {

}

void PollService::handleEvent(const Event& t_event) {
  if (EventToSubCategory(t_event.m_type) == EventSubCategory::kHalSwitchEvent) {
    SwitchId id = static_cast<SwitchId>(t_event.m_data.id);

    if (t_event.m_type == EventType::kSwitchPressed) {
      switch (id) {
        case SwitchId::kBypass:
          EventBus::publish({EventType::kRawBypassPressed, t_event.m_timestamp, {}});
          break;

        case SwitchId::kTap:
          EventBus::publish({EventType::kRawTapPressed, t_event.m_timestamp, {}});
          break;

        case SwitchId::kMenuEncoder:
          EventBus::publish({EventType::kRawMenuEncoderPressed, t_event.m_timestamp, {}});
          break;

        case SwitchId::kProgramMode:
          EventBus::publish({EventType::kRawProgramModeSwitchPress, t_event.m_timestamp, {}});
          break;

        case SwitchId::kMenuLock:
          EventBus::publish({EventType::kRawMenuLockPressed, t_event.m_timestamp, {}});
          break;

        default:
          break;
      }

      return;
    }
    else if (t_event.m_type == EventType::kSwitchLongPressed) {
      switch (id) {
        case SwitchId::kTap:
          EventBus::publish({EventType::kRawTapLongPressed, t_event.m_timestamp, {}});
          break;

        case SwitchId::kMenuEncoder:
          EventBus::publish({EventType::kRawMenuEncoderLongPressed, t_event.m_timestamp, {}});
          break;

        case SwitchId::kProgramMode:
          EventBus::publish({EventType::kRawProgramModeSwitchLongPress, t_event.m_timestamp, {}});
          break;

        case SwitchId::kMenuLock:
          EventBus::publish({EventType::kRawMenuLockLongPressed, t_event.m_timestamp, {}});
          break;

        default:
          break;
      }

      return;
    }
    else if (t_event.m_type == EventType::kSwitchReleased) {
      switch (id) {
        case SwitchId::kTap:
          EventBus::publish({EventType::kRawTapReleased, t_event.m_timestamp, {}});
          break;

        case SwitchId::kMenuEncoder:
          EventBus::publish({EventType::kRawMenuEncoderReleased, t_event.m_timestamp, {}});
          break;

        case SwitchId::kProgramMode:
          EventBus::publish({EventType::kRawProgramModeSwitchReleased, t_event.m_timestamp, {}});
          break;

        case SwitchId::kMenuLock:
          EventBus::publish({EventType::kRawMenuLockReleased, t_event.m_timestamp, {}});
          break;

        default:
          break;
      }

      return;
    }
  }
  else if (EventToSubCategory(t_event.m_type) == EventSubCategory::kHalEncoderEvent) {
    EncoderId id = static_cast<EncoderId>(t_event.m_data.id);

    switch (id) {
      case EncoderId::kMenuEncoder: {
        Event e;
        e.m_type = EventType::kRawMenuEncoderMoved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.delta = t_event.m_data.delta;
        EventBus::publish(e);
        break;
      }

      default:
        break;
    }

    return;
  }
  else if (EventToSubCategory(t_event.m_type) == EventSubCategory::kHalPotEvent) {
    PotId id = static_cast<PotId>(t_event.m_data.id);

    switch(id) {
      case PotId::kPot0: {
        Event e;
        e.m_type = EventType::kRawPot0Moved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.value = t_event.m_data.value;
        EventBus::publish(e);
        break;
      }

      case PotId::kPot1: {
        Event e;
        e.m_type = EventType::kRawPot1Moved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.value = t_event.m_data.value;
        EventBus::publish(e);
        break;
      }

      case PotId::kPot2: {
        Event e;
        e.m_type = EventType::kRawPot2Moved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.value = t_event.m_data.value;
        EventBus::publish(e);
        break;
      }

      case PotId::kMixPot: {
        Event e;
        e.m_type = EventType::kRawMixPotMoved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.value = t_event.m_data.value;
        EventBus::publish(e);
        break;
      }

      case PotId::kExpr: {
        Event e;
        e.m_type = EventType::kRawExprMoved;
        e.m_timestamp = t_event.m_timestamp;
        e.m_data.value = t_event.m_data.value;
        EventBus::publish(e);
        break;
      }

      default:
        break;
    }

    return;
  }
}

void PollService::update() {

}

bool PollService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kHalEvent;
}
