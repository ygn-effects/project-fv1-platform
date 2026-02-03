#include "logic/crossfade_handler.h"

CrossfadeResult CrossfadeHandler::calculate(uint16_t t_mixValue) {
  uint16_t dry = 0;
  uint16_t wet = 0;

  if (t_mixValue > c_maxValue) t_mixValue = c_maxValue;

  switch (m_currentCurve) {
    case MixCurve::kTransition:
      if (t_mixValue < c_midPoint) {
        // 1023 dry
        // 0..512 -> 0..1023 wet

        dry = c_maxValue;
        wet = Utils::mapValue<uint16_t>(t_mixValue, 0, c_midPoint, 0, c_maxValue);
      }
      else {
        // 1023 wet
        // 512..1023 -> 1023..0 dry

        wet = c_maxValue;
        dry = Utils::mapValue<uint16_t>(t_mixValue, c_midPoint, c_maxValue, c_maxValue, 0);
      }

      break;

    case MixCurve::kLinear:
      wet = t_mixValue;
      dry = c_maxValue - t_mixValue;

      break;

    default:
      break;
  }

  return {dry, wet};
}
