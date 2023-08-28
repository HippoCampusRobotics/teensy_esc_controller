#pragma once
#include "pins.hpp"
#include <Arduino.h>
namespace pwm
{
  static constexpr int kPwmResolution = 15;
  static constexpr int kServoNeutralUs = 1500;
  static constexpr int kServoAmplitudeUs = 400;
  static constexpr int kServoMinUs = kServoNeutralUs - kServoAmplitudeUs;
  static constexpr int kServoMaxUs = kServoNeutralUs + kServoAmplitudeUs;
  static constexpr int kServoPulseWidthUs = 20000;
  static constexpr int kPwmPulseWidth = 32757; // 2^15 - 1
  static constexpr int kPwmNeutral = kServoNeutralUs * kPwmPulseWidth / kServoPulseWidthUs;
  static constexpr int kPwmMin = (kServoNeutralUs - kServoAmplitudeUs) * kPwmPulseWidth / kServoPulseWidthUs;
  static constexpr int kPwmMax = (kServoNeutralUs + kServoAmplitudeUs) * kPwmPulseWidth / kServoPulseWidthUs;

  inline int ServoToPwm(int _servo)
  {
    return (kPwmMax - kPwmMin) * (_servo - kServoMinUs) / (2 * kServoAmplitudeUs) + kPwmMin;
  }

  inline void SetPwm(size_t i, int _servo_pulse_us)
  {
    analogWrite(pins::kServo[i], ServoToPwm(_servo_pulse_us));
  }

  inline void StopThrusters()
  {
    for (int i = 0; i < pins::kServoCount; ++i)
    {
      analogWrite(pins::kServo[i], kPwmNeutral);
    }
  }
} // pwm