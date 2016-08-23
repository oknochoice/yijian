#include "macro.h"

#include <bitset>
#include <arpa/inet.h>

static std::bitset<8> & getBitset() {
  YILOG_TRACE(__func__);
  static thread_local std::bitset<8> bitset_8;
  return bitset_8;
};


class packet
  : public yijian::noncopyable {
public:
  // where, length
  typedef std::pair<char *, int> Data;
  typedef std::vector<Data> Payload;
protected:
  size_t write_string(std::string && str, char *& pos);
  Data fixed_header_;
  Data variable_header_;
  Payload payload_;
};

class connect
  : public packet {
public:
  connect(uint16_t keep_alive = 60, 
      bool is_clean_session = false,
      std::string protocol_name = "MQTT", 
      uint8_t mqtt_level = 4) {
    YILOG_TRACE(__func__);

  }
private:
void setup_fixed_header(uint32_t remain_length);
void setup_variable_header(
    std::string && protocol_name, 
    uint8_t mqtt_level,
    bool is_clean_session,
    bool will_flag,
    uint8_t will_qos,
    bool is_will_retain,
    bool password_flag,
    bool username_flag,
    uint16_t keep_alive);
void setup_payload(std::string && client_id,
    std::shared_ptr<std::string> will_topic,
    std::shared_ptr<std::string> will_message,
    std::shared_ptr<std::string> username,
    std::shared_ptr<std::string> password);
};
