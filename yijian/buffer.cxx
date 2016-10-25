#include "buffer.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

namespace yijian {

// buffer 
// construct
buffer::buffer(Message_Type type) 
  : buffer_type_(type) {

  YILOG_TRACE("func: {}", __func__);

  std::size_t init_size = static_cast<std::size_t>(buffer_type_);
  header_pos_ = (char *)malloc(init_size);

  reset();

};

void buffer::reset() {

  data_pos_ = header_pos_;
  current_pos_ = header_pos_;

}
// destruct
buffer::~buffer() {

  YILOG_TRACE("func: {}", __func__);

  free(header_pos_);

}

inline char * buffer::header() {
  YILOG_TRACE("func: {}", __func__);
  return header_pos_;
}

inline std::size_t buffer::size() {
  YILOG_TRACE("func: {}", __func__);
  return current_pos_ - header_pos_;
}

inline std::size_t buffer::remain_size() {
  YILOG_TRACE("func: {}", __func__);
  return static_cast<std::size_t>(buffer_type_) - size();
}

inline Message_Type buffer::buffer_type() {
  YILOG_TRACE("func: {}", __func__);
  return buffer_type_;
}


bool buffer::socket_read(int sfd) {
  YILOG_TRACE("func: {}", __func__);
  // parse length and data type
  if (!isParseFinish_) {
    if (0 != parse_length_) {
      parse_length_ -= socket_read(sfd, parse_length_);
    }else {
      auto pair = decoding_var_Length(header_pos_);
      data_pos_ = pair.second + MESSAGE_TYPE_LENGTH;
      char type = *pair.second;
      data_type_ = type;

      int readed = 4 + MESSAGE_TYPE_LENGTH - (pair.second - header_pos_);

      remain_data_length_ = pair.first - readed;
      isParseFinish_ = true;

      if (remain_data_length_ + 4 + MESSAGE_TYPE_LENGTH > 
          static_cast<std::size_t>(buffer_type_)) {
        throw std::system_error(std::error_code(20000, std::generic_category()),
            "transfer data over buffer size");
      }
    }
  }else if(!isFinish_){// read remain data;

    remain_data_length_ -= socket_read(sfd, remain_data_length_);
    if (0 == remain_data_length_)
      isFinish_ = true;

  }
  return isFinish_;
}

bool buffer::socket_write(int sfd) {
  YILOG_TRACE("func: {}", __func__);
  remain_data_length_ -= socket_write(sfd, remain_data_length_);
  if (0 == remain_data_length_) {
    isFinish_ = true;
  }
  return isFinish_;
}

uint_fast8_t buffer::datatype() {
  YILOG_TRACE("func: {}", __func__);
  return data_type_;
}

char * buffer::data() {
  YILOG_TRACE("func: {}", __func__);
  return data_pos_;
}

std::size_t buffer::data_size() {
  YILOG_TRACE("func: {}", __func__);
  return current_pos_ - data_pos_;
}

std::pair<uint_fast32_t, char *>
buffer::decoding_var_Length(char * pos) {
  YILOG_TRACE("func: {}", __func__);
  int multiplier = 1;
  uint_fast32_t value = 0;
  uint8_t encodeByte;
  do {
    encodeByte = *pos;
    value += (encodeByte & 127) * multiplier;
    ++pos;
    multiplier *= 128;
    if (multiplier > 128 * 128 * 128)
      throw std::system_error(std::error_code(20001, std::generic_category()),
          "Malformed Remaining Length");
  }while((encodeByte & 128) != 0);
  return std::make_pair(value, pos);
}

char *
buffer::encoding_var_Length(char * pos, uint_fast32_t length) {
  YILOG_TRACE("func: {}", __func__);
  do {
    uint8_t encodeByte = length % 128;
    length = length / 128;
    if (length > 128) {
      encodeByte = encodeByte | 128;
    }
    memcpy(pos, &encodeByte, 1);
    ++pos;
  }while (length > 0);
  return pos;
}

inline std::size_t buffer::socket_read(int sfd, std::size_t count) {
  YILOG_TRACE("func: {}", __func__);
  int readed = read(sfd, current_pos_, count);
  if (-1 != readed) {
    current_pos_ += readed;
  }else {
    throw std::system_error(std::error_code(errno, std::system_category()), "read buffer");
  }
  return readed;
}

inline std::size_t buffer::socket_write(int sfd, std::size_t count) {
  YILOG_TRACE("func: {}", __func__);
  int writed = write(sfd, current_pos_, count);
  if (-1 != writed) {
    current_pos_ += writed;
  }else {
    throw std::system_error(std::error_code(errno, std::system_category()), "write buffer");
  }
  return writed;
}

}

