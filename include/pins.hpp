#pragma once

namespace pins
{
  static constexpr int kLed = 13;
  static constexpr int kVoltage = 14;
  static constexpr int kCurrent = 15;
  static constexpr int kServo[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  static constexpr size_t kServoCount = sizeof(kServo) / sizeof(kServo[0]);
}