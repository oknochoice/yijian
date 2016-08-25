#include "message.h"

namespace yijian {


void connect::setup_fixed_header(uint32_t remain_length) {
  YILOG_TRACE(__func__);
  // control packet type
  getBitset().reset();
  getBitset().set(4);
  uint8_t control_type = getBitset().to_ulong();
  fixed_header_.write_8bit(control_type);
  // remain length
  int multiplier = 1;
  int value = 0;
  uint8_t encodeByte;
  do {
    encodeByte = remain_length % 128;
    value += (encodeByte & 127) * multiplier;
    multiplier *= 128;
    if (multiplier > 128 * 128 * 128) 
      throw std::invalid_argument("Malformed Remaining Length");
    fixed_header_.write_8bit(encodeByte);
  }while((encodeByte & 128) != 0);
}

void connect::setup_variable_header(
    std::string && protocol_name, 
    uint8_t mqtt_level,
    bool clean_session,
    bool will_flag,
    uint8_t will_qos,
    bool will_retain,
    bool password_flag,
    bool username_flag,
    uint16_t keep_alive) {
  YILOG_TRACE(__func__);

  // protocol name
  variable_header_.write_16bit(protocol_name.size());
  variable_header_.write(std::forward<std::string>(protocol_name));
  // protocol level
  variable_header_.write_8bit(mqtt_level);
  // connect flags
  getBitset().reset();
  if (clean_session) getBitset().set(2);
  if (will_flag) getBitset().set(3);
  if (1 == will_qos % 2) getBitset().set(4);
  if (1 == will_qos / 2) getBitset().set(5);
  if (will_retain) getBitset().set(6);
  if (password_flag) getBitset().set(7);
  if (username_flag) getBitset().set(8);
  uint8_t connect_flags = getBitset().to_ulong();
  variable_header_.write_8bit(connect_flags);
  // keep alive
  variable_header_.write_16bit(keep_alive);
}


void connect::setup_payload(std::string && client_id,
    std::shared_ptr<std::string> will_topic,
    std::shared_ptr<std::string> will_message,
    std::shared_ptr<std::string> username,
    std::shared_ptr<std::string> password) {
  YILOG_TRACE(__func__);
  // client id
  if (!client_id.empty()) {
    payload_.write_16bit(client_id.size());
    payload_.write(client_id.data());
  }
  if (will_topic && !will_topic->empty()) {
    payload_.write_16bit(will_topic->size());
    payload_.write(will_topic->data());
  }
  if (will_message && !will_message->empty()) {
    payload_.write_16bit(will_message->size());
    payload_.write(will_message->data());
  }
  if (username && !username->empty()) {
    payload_.write_16bit(username->size());
    payload_.write(username->data());
  }
  if (password && !password->empty()) {
    payload_.write_16bit(password->size());
    payload_.write(password->data());
  }

}

}
