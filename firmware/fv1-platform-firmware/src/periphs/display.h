#pragma once

#include <stdint.h>

class DisplayDriver {
  public:
    virtual void init() = 0;
    virtual void clear() = 0;
    virtual void drawRect(int16_t t_x, int16_t t_y, int16_t t_w, int8_t t_h, bool t_filled = true) = 0;
    virtual void drawText(int16_t t_x, int16_t t_y, const char* t_text, bool t_inverted = false) = 0;
    virtual void setTextSize(uint8_t t_size) = 0;
    virtual void display() = 0;

    virtual int16_t getWidth() const = 0;
    virtual int16_t getHeight() const = 0;
    virtual uint16_t getTextWidth(const char* t_text) = 0;
    virtual uint16_t getTextHeight(const char* t_text) = 0;
    virtual uint16_t getLineHeight() = 0;
};
