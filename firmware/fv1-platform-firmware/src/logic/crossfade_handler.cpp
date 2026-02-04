#include "logic/crossfade_handler.h"

CrossfadeResult CrossfadeHandler::calculate(uint16_t t_mixValue) {
  uint16_t dry = 0;
  uint16_t wet = 0;

  if (t_mixValue > c_maxValue) t_mixValue = c_maxValue;

  switch (m_currentCurve) {
    case MixCurve::kTransition:
      if (t_mixValue <= c_midPoint) {
        // Left Side (0 to 512)
        dry = c_maxValue;

        // Map 0..512 input -> 0..1023 output
        wet = Utils::mapValue<uint16_t>(t_mixValue, 0, c_midPoint, 0, c_maxValue);
      }
      else {
        // Right Side (513 to 1023)
        wet = c_maxValue;

        // Map 512..1023 input -> 1023..0 output
        dry = Utils::mapValue<uint16_t>(t_mixValue, c_midPoint, c_maxValue, c_maxValue, 0);
      }

      break;

    case MixCurve::kLinear:
      wet = t_mixValue;
      dry = c_maxValue - t_mixValue;

      break;

    case MixCurve::kConstantPower: {
      // Map to 8 bits
      uint8_t index = t_mixValue >> 2;
      // Map to the sine lut
      wet = sineLut[index];
      dry = sineLut[255 - index];

      break;
    }

    case MixCurve::kLogarithmic: {
      uint32_t square = (uint32_t)t_mixValue * t_mixValue;
      wet = square >> 10;
      dry = 1023 - wet;

      break;
    }

    default:
      break;
  }

  return {dry, wet};
}
