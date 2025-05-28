#pragma once

class DigitalGpio {
  public:
    virtual void init() = 0;
    virtual bool read() = 0;
    virtual void write(bool t_value) = 0;
};

class AnalogGpio {
  public:
    virtual void init() = 0;
    virtual uint16_t read() = 0;
    virtual void write(uint16_t t_value) = 0;
};
