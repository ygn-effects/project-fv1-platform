#pragma once

#include <stdint.h>
#include "core/event.h"

class Service {
  public:
    virtual void init() = 0;
    virtual void handleEvent(const Event& t_event) = 0;
    virtual void update() = 0;
};
