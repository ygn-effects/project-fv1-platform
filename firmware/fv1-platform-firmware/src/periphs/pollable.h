#pragma once

#include <stdint.h>
#include <stddef.h>
#include "core/event.h"

class Pollable {
  public:
    virtual void init() = 0;
    virtual size_t poll(Event* t_outEvents, size_t t_maxEvents) = 0;
};
