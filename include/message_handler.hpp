#pragma once
#include <stdint.h>

#include "esc_serial.hpp"

class MessageHandler {
 public:
  static constexpr uint32_t kTimeOutMs = 1000;
  MessageHandler();
  void HandleMessage(esc_serial::Packet &_packet, esc_serial::msg_id_t _msg_id);
  bool ActuatorControlsTimedOut(uint32_t _now) const {
    return (_now > t_actuator_controls_ms_ + kTimeOutMs);
  }

 private:
  void HandleActuatorControlsMessage(
      const esc_serial::ActuatorControlsMessage &_msg);
  uint32_t t_actuator_controls_ms_{0};
};
