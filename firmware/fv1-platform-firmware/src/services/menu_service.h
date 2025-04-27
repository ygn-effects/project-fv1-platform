#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "ui/menu_model.h"
#include "ui/menu_stack.h"

namespace UiConstants {
  static constexpr uint32_t cMenuTimeout = 30000u;
}

enum class SubState : uint8_t {
  kSelecting,
  kEditing
};

class MenuService : public Service {
  private:


  public:

};
