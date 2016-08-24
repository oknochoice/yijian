#include "buffer.h"
#include <arpa/inet.h>

namespace yijian {

// buffer_loop
buffer_loop::buffer_loop(uint_fast64_t buffer_size, 
    uint_fast32_t buffer_count) 
  : deque_(std::deque<uint8_t*>(buffer_count)), 
    buffer_size_(buffer_size) {
  YILOG_TRACE(__func__);
  alloc(buffer_size_, buffer_count_);
}

uint8_t* buffer_loop::get() {
  YILOG_TRACE(__func__);
  if (!deque_.empty()) {
    if( 5 == block_count_ ) {
      YILOG_CRITICAL("buffer loop's blocks too many to alloc "
                "maybe not malloc");
    }
    YILOG_WARN("need rejust buffer size and buffer count");
    alloc(buffer_size_, buffer_count_);
  }
  uint8_t * p = deque_.front();
  deque_.pop_front();
  return p;
}
inline void buffer_loop::put(uint8_t * pos) {
  YILOG_TRACE(__func__);
  deque_.push_back(pos);
}

void buffer_loop::alloc(uint_fast64_t buffer_size,
                        uint_fast32_t buffer_count) {
  YILOG_TRACE(__func__);
  uint8_t * p = (uint8_t *)std::malloc(buffer_size * buffer_count);
  int_fast32_t i = -1;
  std::generate_n(std::back_insert_iterator<std::deque<uint8_t*> >(deque_),
                  buffer_count_, [=]() mutable {++i; 
                                    return p + i * buffer_size_;});
  ++block_count_;
}

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
// buffer 
//construct
buffer::buffer(message_type msg_type)
  : max_size_(static_cast<uint_fast64_t>(msg_type)), 
    remain_length_(static_cast<uint_fast64_t>(msg_type)),
    msg_type_(msg_type),
    isNeedRelease_(true){
  YILOG_TRACE(__func__);

};
// move constructor
buffer::buffer(buffer&& buf) noexcept{
  YILOG_TRACE(__func__);
  assign(buf);
}

buffer& buffer::operator=(buffer&& buf) noexcept{
  YILOG_TRACE(__func__);
  clean();
  assign(buf);
  return *this;
}

void buffer::assign(buffer& buf) {
  YILOG_TRACE(__func__);
  header_pos_ = buf.header_pos_;
  current_pos_ = buf.current_pos_;
  max_size_ = buf.max_size_;
  remain_length_ = buf.remain_length_;
  isNeedRelease_ = true;
  buf.isNeedRelease_ = false;
}

buffer::Unwrited_Data buffer::write(const uint8_t * pos, uint_fast32_t length) noexcept{
  YILOG_TRACE(__func__);
  uint_fast32_t real_length;
  if (remain_length_ >= length) {
    real_length = length;
  }else {
    real_length = remain_length_;
  }
  std::memcpy(current_pos_, pos, real_length);
  current_pos_ += real_length;
  remain_length_ -= length;
  if (0 == remain_length_) {
    return std::make_pair(pos + real_length, length - real_length);
  }else {
    return std::make_pair(nullptr, 0);
  }
}

inline void buffer::write(uint8_t value) {
  YILOG_TRACE(__func__);
  write(&value, sizeof(value));
}

inline void buffer::write(uint16_t value) {
  YILOG_TRACE(__func__);
  uint16_t net_length = htons(value);
  write(reinterpret_cast<const uint8_t*>(&net_length), sizeof(net_length));
}

void buffer::write(std::string && str) {
  YILOG_TRACE(__func__);
  write(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

inline std::size_t buffer::size() {
  YILOG_TRACE(__func__);
  return current_pos_ - header_pos_;
}

inline void buffer::clean() {
  if (isNeedRelease_) {
    get_loop(msg_type_).put(header_pos_);
  }
}
// buffers

buffers::buffers(buffers&& bufs) noexcept {
  assign(bufs);
}

buffers& buffers::operator=(buffers&& bufs) noexcept {
  assign(bufs);
  return *this;
}
void buffers::assign(buffers & bufs) {
  array_ = bufs.array_;
  pos_ = bufs.pos_;
  msg_type_ = bufs.msg_type_;
}

buffers::buffers(message_type msg_type)
  : array_(std::vector<buffer>(5)), 
    msg_type_(msg_type) {
  YILOG_TRACE(__func__);
  array_.push_back(buffer(msg_type_));
};

void buffers::write(uint8_t * pos, uint_fast64_t length) {
  YILOG_TRACE(__func__);
  auto * buf = &array_.back();
  const uint8_t * current_pos_ = pos;
  do {
    auto pair = buf->write(current_pos_, length);
    if (0 != pair.second) {
      array_.push_back(buffer(msg_type_));
      buf = &array_.back();
      current_pos_ = pair.first;
    }else {
      break;
    }
  }while (true);
}

}
