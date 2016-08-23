#include <macro.h>
#include <deque>

namespace yijian {


enum class message_type : uint_fast64_t{
  fixed_header = 5,
  variable_header = 10,
  small_message = 128,
  message = 1024,
  image = 1024 * 1024 * 256,
  big_image = 1024 * 1024 * 1024
};

class buffer 
  : public yijian::noncopyable{
public:
    typedef std::pair<char * , uint_fast32_t> Unwrited_Data;
    
    //construct
    buffer(char * header, uint_fast32_t length) 
      : header_pos_(header), current_pos_(header),
        max_size_(length), remain_length_(length){
      YILOG_TRACE(__func__);
    };

    // member func
    Unwrited_Data write(char * pos, uint_fast32_t length) noexcept;
    char * header();
    std::size_t size();
private:
    char * header_pos_;
    char * current_pos_;
    uint_fast32_t max_size_;
    uint_fast32_t remain_length_;
};

class buffer_loop
  : public yijian::noncopyable {
public:
    //construct
    buffer_loop(uint_fast64_t buffer_size, 
        uint_fast32_t buffer_count) 
      : deque_(std::deque<char*>(buffer_count)), 
        buffer_size_(buffer_size) {
      YILOG_TRACE(__func__);
      alloc();
    }
    // member func
    buffer get();
    void put(buffer && buf);
private:
    void alloc();
private:
    std::deque<char *> deque_;
    uint_fast64_t buffer_size_;
    uint_fast32_t buffer_count_;
    uint_fast32_t block_count_ = 0;
};


static buffer_loop& get_loop(message_type msg_type) {
  YILOG_TRACE(__func__);
  switch(msg_type) {
    case message_type::fixed_header: 
      static thread_local buffer_loop loop1 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop1;
    case message_type::variable_header: 
      static thread_local buffer_loop loop2 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop2;
    case message_type::small_message: 
      static thread_local buffer_loop loop3 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop3;
    case message_type::message: 
      static thread_local buffer_loop loop4 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop4;
    case message_type::image: 
      static thread_local buffer_loop loop5 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop5;
    case message_type::big_image: 
      static thread_local buffer_loop loop6 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop6;
  }
}

class buffers 
  : public yijian::noncopyable{
public:
    //construct
    buffers(message_type msg_type): array_(std::vector<buffer>(5)), 
        m_type_(msg_type) {
      YILOG_TRACE(__func__);
      array_.push_back(get());
    };
    //member func
    void write(char * pos, uint_fast32_t length);
private:
    buffer get();
private:
    std::vector<buffer> array_;
    char * pos_;
    message_type m_type_;
};

}

