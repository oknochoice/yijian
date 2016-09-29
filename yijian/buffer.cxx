#include "buffer.h"
#include <arpa/inet.h>

namespace yijian {

// buffer 
// construct
buffer::buffer() {

  YILOG_TRACE(__func__);

  header_pos_ = malloc()

};

buffer::~buffer() {

  YILOG_TRACE(__func__);

}

bool buffer::setIntegritySize(std::size_t size) {
  if (size >= buffer::size()) {
    integrity_size_ = size;
    return true;
  }else {
    return false;
  }
}

bool buffer::isIntegrity() {
  if (0 != integrity_size_ &&
      size() == integrity_size_) return true;
  else return false;
}

buffer::Unwrited_Data buffer::write(const uint8_t * pos, std::size_t length) noexcept{
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

inline std::size_t buffer::size() {
  YILOG_TRACE(__func__);
  return current_pos_ - header_pos_;
}

inline std::size_t buffer::writable_size() {
  YILOG_TRACE(__func__);
  return remain_length_;
}

}
