#include "message.h"

void packet::write_string(std::string && str, char *& pos) {
  uint16_t net_length = htons(str.length());
  std::memcpy(pos, &net_length, sizeof(net_length));
  pos += sizeof(net_length);
  std::memcpy(pos, str.data(), str.size());
  pos += str.size();
}

void connect::setup_fixed_header(uint32_t remain_length) {
  YILOG_TRACE(__func__);

  // fixed header
  char * fixed_header_pos = *(fixed_header_.first);
  // control packet type
  getBitset().reset();
  getBitset().set(4);
  uint8_t control_type = getBitset().to_ulong();
  std::memcpy(fixed_header_pos, &control_type, sizeof(control_type));
  fixed_header_pos += sizeof(control_type);
  // remain length
  int multiplier = 1;
  int value = 0;
  uint8_t encodeByte;
  int count = 0;
  do {
    encodeByte = remain_length % 128;
    value += (encodeByte & 127) * multiplier;
    multiplier *= 128;
    if (multiplier > 128 * 128 * 128) 
      throw std::invalid_argument("Malformed Remaining Length");
    count++;
    std::memcpy(fixed_header_pos, &encodeByte, 1);
    fixed_header_pos++;
  }while((encodeByte & 128) != 0);
  fixed_header_.second = 1 + count;
  fixed_header_pos = nullptr;
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
  // variable header
  char * var_header_pos = *(variable_header_.first);
  // protocol name
  write_string(std::forward<std::string>(protocol_name), var_header_pos);
  // protocol level
  std::memcpy(var_header_pos, &mqtt_level, sizeof(mqtt_level));
  var_header_pos += sizeof(mqtt_level);
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
  std::memcpy(var_header_pos, &connect_flags, sizeof(connect_flags));
  var_header_pos += sizeof(connect_flags);
  // keep alive
  uint16_t keep_alive_net = htons(keep_alive);
  std::memcpy(var_header_pos, &keep_alive_net, sizeof(keep_alive_net));
  var_header_pos = nullptr;
}


void connect::setup_payload(std::string && client_id,
    std::shared_ptr<std::string*> will_topic,
    std::shared_ptr<std::string*> will_message,
    std::shared_ptr<std::string*> username,
    std::shared_ptr<std::string*> password) {
  YILOG_TRACE(__func__);
  // payload
  char * payload_pos = *(payload_.first);
  write_string(std::forward<std::string>(client_id),payload_pos)  ;
}
