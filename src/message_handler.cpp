#include "message_handler.hpp"

#include <Arduino.h>

#include "pins.hpp"
#include "pwm.hpp"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

MessageHandler::MessageHandler() { t_actuator_controls_ms_ = millis(); }

void MessageHandler::HandleMessage(esc_serial::Packet &_packet, esc_serial::msg_id_t _msg_id) {
  switch (_msg_id) {
    case esc_serial::ActuatorControlsMessage::MSG_ID:
      esc_serial::ActuatorControlsMessage msg;
      msg.Deserialize(_packet.PayloadStart(), _packet.PayloadSize());
      HandleActuatorControlsMessage(msg);
      break;
  }
}

void MessageHandler::HandleActuatorControlsMessage(
    const esc_serial::ActuatorControlsMessage &_msg) {
  t_actuator_controls_ms_ = millis();
  for (size_t i = 0; i < ARRAY_LENGTH(_msg.payload_.pwm); ++i) {
    pwm::SetPwm(i, _msg.payload_.pwm[i]);
  }
}
