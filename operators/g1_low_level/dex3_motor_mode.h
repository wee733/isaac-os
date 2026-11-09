/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ISAAC_OS_OPERATORS_G1_LOW_LEVEL_DEX3_MOTOR_MODE_H
#define ISAAC_OS_OPERATORS_G1_LOW_LEVEL_DEX3_MOTOR_MODE_H

#include <cstdint>

namespace isaac::os::ops {

/// Unitree Dex3 protocol: motor ID in bits 0-3, status in bits 4-6, timeout in bit 7.
/// Matches make_hand_mode() in unitree_sdk2_python's g1_move_hands_example.py.
constexpr std::uint8_t dex3_motor_mode(std::uint8_t motor_index) {
  constexpr std::uint8_t kEnabled = 0x10U;
  constexpr std::uint8_t kTimeout = 0x80U;
  return static_cast<std::uint8_t>((motor_index & 0x0FU) | kEnabled | kTimeout);
}

}  // namespace isaac::os::ops

#endif  // ISAAC_OS_OPERATORS_G1_LOW_LEVEL_DEX3_MOTOR_MODE_H
