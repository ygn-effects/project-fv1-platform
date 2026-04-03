#include <Arduino.h>
#include <SPI.h>
#include "periphs/dac.h"
#include "periphs/gpio.h"

namespace hal {

enum class Type : uint8_t {
  kSingleChannel,
  kDualChannel
};

enum class BufferMode : uint8_t {
  kUnbuffered,
  kBuffered
};

enum class GainMode : uint8_t {
  kGain2x,
  kGain1x
};

enum class ShutdownMode : uint8_t {
  kShutdown,
  kActive
};

enum class Channel : uint8_t {
  kA,
  kB
};

class Mcp49xx : public Dac {
  protected:
    DigitalGpio& m_csPin;

    void select();
    void deselect();

    uint16_t makeDacWord(uint16_t t_data12, Type t_type, BufferMode t_buffer, GainMode t_gain, ShutdownMode t_shutdown, Channel t_channel = Channel::kA);
    uint16_t make8bitDacWord(uint8_t t_data8, Type t_type, BufferMode t_buffer, GainMode t_gain, ShutdownMode t_shutdown, Channel t_channel = Channel::kA);
    uint16_t make10bitDacWord(uint16_t t_data10, Type t_type, BufferMode t_buffer, GainMode t_gain, ShutdownMode t_shutdown, Channel t_channel = Channel::kA);

  public:
    Mcp49xx(DigitalGpio& t_csPin);

    void init() override;
};

class Mcp4911 : public Mcp49xx {
  public:
    explicit Mcp4911(DigitalGpio& t_csPin);

    void write(uint16_t t_value) override;
};

class Mcp4912 : public Mcp49xx {
  private:
  Channel m_channel;

  public:
    explicit Mcp4912(DigitalGpio& t_csPin, Channel m_channel);

    void write(uint16_t t_value) override;
};

}
