#include "logic/crossfade_handler.h"

CrossfadeResult CrossfadeHandler::calculate(uint16_t t_mixValue) {
  uint16_t dry = 0;
  uint16_t wet = 0;

  if (t_mixValue > c_maxValue) t_mixValue = c_maxValue;
  uint16_t effectiveValue = Utils::mapValue<uint16_t>(t_mixValue, 0, c_maxValue, m_minInputValue, m_maxInputValue);

  switch (m_currentCurve) {
    case MixCurve::kTransition:
      if (effectiveValue <= c_midPoint) {
        // Left Side (0 to 512)
        dry = c_maxValue;

        // Map 0..512 input -> 0..1023 output
        wet = Utils::mapValue<uint16_t>(effectiveValue, 0, c_midPoint, 0, c_maxValue);
      }
      else {
        // Right Side (513 to 1023)
        wet = c_maxValue;

        // Map 512..1023 input -> 1023..0 output
        dry = Utils::mapValue<uint16_t>(effectiveValue, c_midPoint, c_maxValue, c_maxValue, 0);
      }

      break;

    case MixCurve::kLinear:
      wet = effectiveValue;
      dry = c_maxValue - effectiveValue;

      break;

    case MixCurve::kConstantPower: {
      // Map to 8 bits
      uint8_t index = effectiveValue >> 2;
      // Map to the sine lut
      wet = sineLut[index];
      dry = sineLut[255 - index];

      break;
    }

    case MixCurve::kLogarithmic: {
      uint32_t square = (uint32_t)effectiveValue * effectiveValue;
      wet = square >> 10;
      dry = 1023 - wet;

      break;
    }

    default:
      break;
  }

  return {dry, wet};
}
