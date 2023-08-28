#include <Arduino.h>

#include "esc_serial.hpp"
#include "message_handler.hpp"
#include "pins.hpp"
#include "pwm.hpp"

#define DEBUG_SERIAL Serial
#if TEENSY_BOARD == 40
#define UART_SERIAL Serial5
#define UART_EVENT serialEvent5
#elif TEENSY_BOARD == 31
#define UART_SERIAL Serial2
#define UART_EVENT serialEvent2
#else
#error "No valid value for TEENSY_BOARD set.
#endif

static constexpr uint32_t kAdcPublishPeriodMs = 1000;
static constexpr float kVref = 3.3;
static constexpr float kVdivider = 11.0;

esc_serial::Packet packet_rx;
esc_serial::Packet packet_tx;
MessageHandler message_handler;
uint32_t t_last_voltage;

inline void ToggleLed() {
  static bool led_state{false};
  digitalWriteFast(pins::kLed, led_state);
  led_state = !led_state;
}

void UART_EVENT() {
  while (UART_SERIAL.available()) {
    if (!packet_rx.AddByte(UART_SERIAL.read())) {
      DEBUG_SERIAL.println("Buffer full. Resetting...");
      packet_rx.Reset();
      continue;
    }
    if (packet_rx.CompletelyReceived()) {
      DEBUG_SERIAL.println("Packet complete.");
      esc_serial::msg_id_t msg_id = packet_rx.ParseMessage();
      DEBUG_SERIAL.print("Message ID: ");
      DEBUG_SERIAL.print(msg_id);
      DEBUG_SERIAL.print('\n');
      message_handler.HandleMessage(packet_rx, msg_id);
      packet_rx.Reset();
      continue;
    }
  }
}

void setup() {
  delay(1000);
  DEBUG_SERIAL.begin(115200);
  UART_SERIAL.begin(115200);
  pinMode(pins::kLed, OUTPUT);
  pinMode(pins::kVoltage, INPUT_DISABLE);
  pinMode(pins::kCurrent, INPUT_DISABLE);
  analogReadResolution(12);
  for (size_t i = 0; i < pins::kServoCount; ++i) {
    analogWriteFrequency(pins::kServo[i], 50);
  }
  analogWriteResolution(pwm::kPwmResolution);
  t_last_voltage = millis();
  pwm::StopThrusters();
}

void loop() {
  static int counter = 0;
  static float voltage{0.0};
  voltage = 0.9 * voltage +
            0.1 * analogRead(pins::kVoltage) * kVref / 4095 * kVdivider;
  uint32_t now = millis();
  if (message_handler.ActuatorControlsTimedOut(now)) {
    counter = (counter + 1) % 10000;
    pwm::StopThrusters();
    if (!counter) {
      DEBUG_SERIAL.println("Setpoint timed out. Stopping thrusters.");
    }
  }
  if (now > t_last_voltage + kAdcPublishPeriodMs) {
    t_last_voltage = now;
    esc_serial::BatteryVoltageMessage msg;
    
    msg.payload_.voltage_mv = (uint16_t)(voltage * 1000.0);
    size_t size = msg.Serialize(packet_tx.MutablePayloadStart(),
                                packet_tx.PayloadCapacity());
    packet_tx.SetPayloadSize(size);
    packet_tx.Packetize();
    UART_SERIAL.write(packet_tx.Data(), packet_tx.Size());
    DEBUG_SERIAL.println("Packet transmitted.");
  }
}
