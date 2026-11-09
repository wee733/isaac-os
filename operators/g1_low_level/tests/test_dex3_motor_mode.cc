/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <array>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "operators/g1_low_level/dex3_motor_mode.h"

namespace isaac::os::ops {
namespace {

TEST(Dex3MotorMode, MatchesUnitreeWireValuesForEachFingerMotor) {
  // Unitree's ID/status/timeout protocol with enabled status and timeout set to one.
  constexpr std::array<std::uint8_t, 7> expected{0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96};
  for (std::size_t index = 0; index < expected.size(); ++index) {
    EXPECT_EQ(dex3_motor_mode(static_cast<std::uint8_t>(index)), expected[index])
        << "Hand motor " << index;
  }
}

}  // namespace
}  // namespace isaac::os::ops
