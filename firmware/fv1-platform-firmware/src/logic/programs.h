#pragma once
#include "logic/program.h"

namespace ProgramsDefinitions {

constexpr Program kPrograms[ProgramConstants::c_maxPrograms]{
  /* ─────────────────── 1 ─ Clean digital delay ─────────────────── */
  {
    "Digital delay",
    {
      /* label,          min, max, fine, coarse, unit,       editable */
      { "Delay time",      20,  1000, 1, 5, ParamUnit::kMs,      true },
      { "Feedback",         0,   100, 1, 5, ParamUnit::kPercent, true },
      { "Tone LPF",       100, 10000, 1, 5, ParamUnit::kHz,      true },
      { "Mix",            0,     100, 1, 5, ParamUnit::kPercent, true }
    },
    true,   /* isDelayEffect   */
    20,     /* minDelayMs      */
    1000,   /* maxDelayMs      */
    true,   /* supportsTap     */
    true    /* supportsExpr    */
  },

  /* ─────────────────── 2 ─ Analog / BBD delay ─────────────────── */
  {
    "Analog delay",
    {
      { "Delay time",     100,  800, 1, 5, ParamUnit::kMs,      true },
      { "Feedback",         0,  100, 1, 5, ParamUnit::kPercent, true },
      { "Clock-grit",       0,  100, 1, 5, ParamUnit::kPercent, true },
      { "Mix",              0,  100, 1, 5, ParamUnit::kPercent, true }
    },
    true,
    100,
    800,
    true,
    true
  },

  /* ─────────────────── 3 ─ Oil-can lo-fi delay ─────────────────── */
  {
    "Oil-can delay",
    {
      { "Delay time",      40,  400, 1,  5, ParamUnit::kMs,      true },
      { "Wobble depth",     0,  100, 1,  5, ParamUnit::kPercent, true },
      { "Band-centre",    500, 3000, 5, 25, ParamUnit::kHz,      true },
      { "Mix",              0,  100, 1,  5, ParamUnit::kPercent, true }
    },
    true,
    40,
    400,
    true,
    true
  },

  /* ─────────────────── 4 ─ Ducked digital delay ─────────────────── */
  {
    "Ducked delay",
    {
      { "Delay time",      80,  750, 1, 5, ParamUnit::kMs,      true },
      { "Feedback",         0,  100, 1, 5, ParamUnit::kPercent, true },
      { "Duck thresh",      0,  100, 1, 5, ParamUnit::kPercent, true },
      { "Mix",              0,  100, 1, 5, ParamUnit::kPercent, true }
    },
    true,
    80,
    750,
    true,
    true
  },

  /* ─────────────────── 5 ─ Tape-echo + spring verb ─────────────────── */
  {
    "Tape echo + spring",
    {
      { "Head spacing",    30,  200, 1, 5, ParamUnit::kMs,      true },
      { "Feedback",         0,  100, 1, 5, ParamUnit::kPercent, true },
      { "Verb mix",         0,  100, 1, 5, ParamUnit::kPercent, true },
      { "Mix",              0,  100, 1, 5, ParamUnit::kPercent, true }
    },
    true,
    30,
    200,
    true,
    true
  },

  /* ─────────────────── 6 ─ Dattorro plate reverb ─────────────────── */
  {
    "Plate reverb",
    {
      { "Decay",          500, 5000, 10, 50, ParamUnit::kMs,      true },
      { "Pre-delay",        0,  100,  1,  5, ParamUnit::kMs,      true },
      { "Damp freq",      200, 8000, 10, 50, ParamUnit::kHz,      true },
      { "Mix",              0,  100,  1,  5, ParamUnit::kPercent, true }
    },
    false,
    0,
    0,
    false,
    true
  },

  /* ─────────────────── 7 ─ Lo-fi room / “dark & drippy” ──────────── */
  {
    "Lo-fi room",
    {
      { "Size",            60, 120, 1, 5, ParamUnit::kMs,      true },
      { "Darkness",         3,   6, 1, 1, ParamUnit::kHz,      true },
      { "Drip",             0, 100, 1, 5, ParamUnit::kPercent, true },
      { "Mix",              0, 100, 1, 5, ParamUnit::kPercent, true }
    },
    false,
    0,
    0,
    false,
    true
  },

  /* ─────────────────── 8 ─ Shimmer plate (freeze) ────────────────── */
  {
    "Shimmer plate",
    {
      { "Decay",          500, 8000,10,50, ParamUnit::kMs,      true },
      { "Shimmer lvl",      0,  100, 1, 5, ParamUnit::kPercent, true },
      { "Freeze",           0,  100, 1, 5, ParamUnit::kPercent, true }, /* >95% engages hold */
      { "Mix",              0,  100, 1, 5, ParamUnit::kPercent, true }
    },
    false,
    0,
    0,
    false,
    true
  }
};

} // namespace ProgramsDefinitions
