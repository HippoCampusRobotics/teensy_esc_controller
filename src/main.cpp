#include <Arduino.h>
#include "packet.hpp"

// neutral: 7.5 %

static constexpr int kServoNeutral = 1500;
static constexpr int kServoAmplitude = 400;
static constexpr int kServoMin = kServoNeutral - kServoAmplitude;
static constexpr int kServoMax = kServoNeutral + kServoAmplitude;
static constexpr int kServoPulseWidth = 20000;
static constexpr int kPwmPulseWidth = 32757; // 2^15 - 1
static constexpr int kPwmNeutral = kServoNeutral * kPwmPulseWidth / kServoPulseWidth;
static constexpr int kPwmMin = (kServoNeutral - kServoAmplitude) * kPwmPulseWidth / kServoPulseWidth;
static constexpr int kPwmMax = (kServoNeutral + kServoAmplitude) * kPwmPulseWidth / kServoPulseWidth;

static constexpr int kPwmResolution = 15;
static constexpr int kNumberMotors = 8;
static constexpr uint32_t kTimeOutMs = 1000;

static constexpr uint32_t kAdcPublishPeriodMs = 1000;
static constexpr float kVref = 3.3;
static constexpr float kVdivider = 10.0;

static constexpr int kLedPin = 13;
static constexpr int kAdcPin = 14;
static constexpr int kServoPins[8] = {0, 1, 2, 3, 4, 5, 6, 7};

uint16_t pwm[kNumberMotors];
Packet pwm_packet;
Packet voltage_packet;
uint32_t t_valid_packet_ms;
uint32_t t_adc_published_ms;

union Voltage
{
  float value;
  uint8_t bytes[sizeof(float)];
};


inline void ToggleLed() {
  static bool led_state{false};
  digitalWriteFast(kLedPin, led_state);
  led_state = !led_state;
}

inline int ServoToPwm(int _servo) {
  return (kPwmMax - kPwmMin) * (_servo - kServoMin) / (2*kServoAmplitude) + kPwmMin;
}


inline void decode_pwm(const uint8_t *_pwm_data) {
  for (int i = 0; i < kNumberMotors; ++i) {
    pwm[i] = *_pwm_data++ << 8;
    pwm[i] |= *_pwm_data++;
  }
}

inline void SetPwm() {
  for (int i=0; i<kNumberMotors; ++i) {
    analogWrite(kServoPins[i], ServoToPwm(pwm[i]));
  }
}

inline void StopThrusters() {
  for (int i=0; i<kNumberMotors; ++i) {
    analogWrite(kServoPins[i], kPwmNeutral);
  }
}
void serialEvent() {
  while (Serial.available()) {
    if (!pwm_packet.AddByte(Serial.read())) {
      // Serial.println("Buffer full. Resetting...");
      pwm_packet.Reset();
    } else if (pwm_packet.Complete()) {
      // Serial.println("Packet complete.");
      if (!pwm_packet.Decode()) {
        // Serial.println("Failed to decode packet.");
      }
      if (pwm_packet.CrcOk()) {
        ToggleLed();
        decode_pwm(pwm_packet.Payload());
        SetPwm();
        t_valid_packet_ms = millis();
      } else {
        // Serial.println("CRC failed.");
      }
      pwm_packet.Reset();
    }
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial) {
  };
  pinMode(kLedPin, OUTPUT);
  pinMode(kAdcPin, INPUT_DISABLE);
  analogReadResolution(12);
  t_valid_packet_ms = millis();
  t_adc_published_ms = millis();
  for (int i=0; i<kNumberMotors; ++i) {
    analogWriteFrequency(kServoPins[i], 50);
  }
  analogWriteResolution(kPwmResolution);
}

void loop() {
  static Voltage voltage{0.0};
  voltage.value = 0.9 * voltage.value + 0.1 * analogRead(kAdcPin) * kVref / 4095 * kVdivider;
  uint32_t now = millis();
  if (now > t_valid_packet_ms + kTimeOutMs) {
    StopThrusters();
  }
  if (now > t_adc_published_ms + kAdcPublishPeriodMs) {
    t_adc_published_ms = now;
    voltage_packet.SetPayload(voltage.bytes, sizeof(voltage.bytes));
    voltage_packet.Packetize();
    Serial.write(voltage_packet.Data(), voltage_packet.Size());
  }
}
