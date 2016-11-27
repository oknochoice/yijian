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
  YILOG_DEBUG("func: {}, size: {}", __func__, init_size);

  header_pos_ = (char *)malloc(init_size);

  reset();

};

void buffer::reset() {

  YILOG_TRACE("func: {}", __func__);

  data_pos_ = header_pos_;
  current_pos_ = header_pos_ + SESSIONID_LENGTH;
  isFinish_ = false;
  isParseFinish_ = false;

}
// destruct
buffer::~buffer() {

  YILOG_TRACE("func: {}", __func__);

  free(header_pos_);

}

char * buffer::header() {
  YILOG_TRACE("func: {}", __func__);
  return header_pos_;
}

std::size_t buffer::size() {
  YILOG_TRACE("func: {}", __func__);
  return current_pos_ - header_pos_;
}

std::size_t buffer::remain_size() {
  YILOG_TRACE("func: {}", __func__);
  return static_cast<std::size_t>(buffer_type_) - size();
}

Message_Type buffer::buffer_type() {
  YILOG_TRACE("func: {}", __func__);
  return buffer_type_;
}


bool buffer::socket_read(int sfd) {
  YILOG_TRACE("func: {}", __func__);
  // parse length and data type
  if (!isParseFinish_) {
    if (0 != parse_length_) {
      parse_length_ -= socket_read(sfd, parse_length_);
      YILOG_DEBUG ("func: {}, parse length: {}", __func__, parse_length_);
    }else {
      // session id
      session_id_ = *header_pos_;
      // var_length
      auto pair = decoding_var_length(header_pos_ + SESSIONID_LENGTH);
      data_pos_ = pair.second;
      data_type_ =  *pair.second;
      YILOG_DEBUG ("func: {}, type: {}, length: {}", 
          __func__, data_type_, pair.first);

      int readed = PADDING_LENGTH - (pair.second - header_pos_);

      remain_data_length_ = pair.first - readed;
      isParseFinish_ = true;

      if (remain_data_length_ + PADDING_LENGTH > 
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


/*
void buffer::set_socketreadmedia_length(std::size_t length) {
  remain_data_length_ = length;
}

bool buffer::socket_read_media(int sfd) {
  YILOG_TRACE("func: {}", __func__);

  if (!isFinish_) {
    remain_data_length_ -= socket_read(sfd, remain_data_length_);
    if (0 == remain_data_length_)
      isFinish_ = true;
  }

  return isFinish_;

}
*/



uint8_t buffer::datatype() {
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


void buffer::data_encoding_length(uint32_t length) {
  current_pos_ = encoding_var_length(current_pos_, length);
}
void buffer::data_encoding_type(uint8_t type) {
  memcpy(current_pos_, &type, 1);
  ++current_pos_;
}
char * buffer::data_encoding_current() {
  return current_pos_;
}
void buffer::data_encoding_current_addpos(std::size_t length) {
  current_pos_ += length;
}

uint16_t buffer::session_id() {
  return session_id_;
}
void buffer::set_sessionid(uint16_t sessionid) {
  memcpy(header_pos_, &sessionid, 2);
}

std::pair<uint32_t, char *>
buffer::decoding_var_length(char * pos) {
  YILOG_TRACE("func: {}", __func__);
  int multiplier = 1;
  uint32_t value = 0;
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
buffer::encoding_var_length(char * pos, uint32_t length) {
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

std::size_t buffer::socket_read(int sfd, std::size_t count) {
  YILOG_TRACE("func: {}", __func__);
  int readed = read(sfd, current_pos_, count);
  YILOG_DEBUG("func: {}, readed: {}", __func__, readed);
  if (0 < readed) {
    current_pos_ += readed;
    return readed;
  }else if (0 == readed) {
    current_pos_ += count;
    return count;
  }else {
    throw std::system_error(std::error_code(errno, std::system_category()),
        "read buffer");
  }
}

std::size_t buffer::socket_write(int sfd, std::size_t count) {
  YILOG_TRACE("func: {}", __func__);
  int writed = write(sfd, current_pos_, count);
  if (-1 != writed) {
    current_pos_ += writed;
  }else {
    throw std::system_error(std::error_code(errno, std::system_category()), 
        "write buffer");
  }
  return writed;
}

}

