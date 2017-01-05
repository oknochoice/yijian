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
  YILOG_TRACE("func: {}, size: {}", __func__, init_size);

  header_pos_ = (char *)malloc(init_size);

  reset();

};

void buffer::reset() {

  YILOG_TRACE("func: {}", __func__);

  data_pos_ = header_pos_;
  end_pos_ = header_pos_;
  current_pos_ = header_pos_;
  isFinish_ = false;
  isParseFinish_ = false;

}
// destruct
buffer::~buffer() {

  YILOG_TRACE("func: {}", __func__);
  YILOG_DEBUG("func: {}", __func__);

  free(header_pos_);

}

char * buffer::header() {
  YILOG_TRACE("func: {}", __func__);
  return header_pos_;
}

std::size_t buffer::size() {
  YILOG_TRACE("func: {}", __func__);
  return end_pos_ - header_pos_;
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
  YILOG_TRACE("func: {}, isParseFinish_: {}, isFinish_: {}", 
      __func__, isParseFinish_, isFinish_);
  if (!isParseFinish_) {
    if (false == isParseMsgReaded_) {
      YILOG_TRACE ("func: {}, read header", __func__);
      int readed = socket_read(sfd, current_pos_, parse_length_);
      current_pos_ += readed;
      parse_length_ -= readed;
      int loopCount = current_pos_ - header_pos_ - SESSIONID_LENGTH - MSG_TYPE_LENGTH;
      YILOG_DEBUG ("loopCount:{}", loopCount);
      if (loopCount > 0) {
        for (int i = 0; i < loopCount; ++i) {
          uint8_t checkbit = *(header_pos_ + SESSIONID_LENGTH + VAR_LENGTH + i);
          YILOG_DEBUG ("checkbit:{}", checkbit);
          if (checkbit >> 7 == 0) {
            isParseMsgReaded_ = true;
            break;
          }
        }
      }
      YILOG_TRACE ("func: {}, parse_length_ : {}"
          "isParseMsgReaded_: {}", 
          __func__, parse_length_, isParseMsgReaded_);
    }
    if (true == isParseMsgReaded_) {
      YILOG_TRACE ("func: {}, parse header", __func__);
      // session id
      session_id_ = *header_pos_;
      // type
      data_type_ =  *(header_pos_ + SESSIONID_LENGTH);
      // var_length
      auto pair = decoding_var_length(header_pos_ + SESSIONID_LENGTH + MSG_TYPE_LENGTH);
      data_pos_ = pair.second;

      YILOG_TRACE ("func: {}, type: {}, length: {}", 
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
    YILOG_TRACE ("func: {}, read reamin date remain_data_length_: {}", 
        __func__, remain_data_length_);

    int readed = socket_read(sfd, current_pos_, remain_data_length_);
    current_pos_ += readed;
    end_pos_ = current_pos_;
    remain_data_length_ -= readed;
    if (0 == remain_data_length_)
      isFinish_ = true;

  }

  YILOG_TRACE("func: {} read finish {}", __func__, isFinish_);
  return isFinish_;
}

bool buffer::socket_write(int sfd) {
  YILOG_TRACE("func: 1 argm {}", __func__);
  YILOG_TRACE("func: 1 argm {}, remain length{}", __func__, remain_data_length_);
  remain_data_length_ -= socket_write(sfd, current_pos_, remain_data_length_);
  YILOG_TRACE("func: 1 argm {}, remain length{}", __func__, remain_data_length_);
  if (0 == remain_data_length_) {
    isFinish_ = true;
    YILOG_TRACE("func: 1 argm {} set finish true", __func__);
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
  return end_pos_ - data_pos_;
}


uint16_t buffer::session_id() {
  return session_id_;
}
void buffer::set_sessionid(uint16_t sessionid) {
  memcpy(header_pos_, &sessionid, 2);
  session_id_ = sessionid;
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
    YILOG_TRACE("func: {} value {}", __func__, value);
#warning limit:16,384 max:268,435,455 i.e multiplier > 128 * 128 * 128 * 128
    if (multiplier > 128 * 128)
      throw std::system_error(std::error_code(20001, std::generic_category()),
          "Malformed Remaining Length");
  }while((encodeByte & 128) != 0);
  YILOG_TRACE("func: {}, return", __func__);
  return std::make_pair(value, pos);
}

char *
buffer::encoding_var_length(char * pos, uint32_t length) {
  YILOG_TRACE("func: {}", __func__);
  do {
    uint8_t encodeByte = length % 128;
    length = length / 128;
    if (length > 0) {
      encodeByte = encodeByte | 128;
    }
    YILOG_TRACE("func: {} encodeByte {}", __func__, encodeByte);
    memcpy(pos, &encodeByte, 1);
    ++pos;
  }while (length > 0);
  return pos;
}

std::size_t buffer::socket_read(int sfd, char * pos, std::size_t count) {
  YILOG_TRACE("func: {}", __func__);
  int readed = read(sfd, pos, count);
  if (0 < readed) {
    YILOG_TRACE("func: {}, readed: {}", __func__, readed);
    noread_count_ = 0;
  }else if (0 == readed) {
    YILOG_TRACE("func: {}, read end", __func__);
    ++noread_count_;
  }else {
    if (EAGAIN == errno) {
      readed = 0;
      noread_count_ = 0;
      YILOG_TRACE("func: {}, errno EAGAIN", __func__);
    }else {
      YILOG_ERROR("func: {}, errno: {}", __func__, errno);
      throw std::system_error(std::error_code(20002, std::system_category()),
        "read buffer");
    }
  }
  if (unlikely(noread_count_ > 3)) {
    YILOG_ERROR("func: {} too much read on noread", __func__);
    throw std::system_error(std::error_code(20003, std::system_category()),
      "too much read on noread");
  }
  return readed;
}

std::size_t buffer::socket_write(int sfd, char * pos, std::size_t count) {
  YILOG_TRACE("func: 3 argm{}", __func__);
  int writed = write(sfd, pos, count);
  YILOG_TRACE("func: 3 argm{}, writed {}", __func__, writed);
  if (-1 != writed) {
  }else {
    if (EAGAIN == errno) {
      writed = 0;
      YILOG_TRACE("func: {}, errno EAGAIN", __func__);
    }else {
      YILOG_ERROR("func: {}, errno: {}", __func__, errno);
      throw std::system_error(std::error_code(20004, std::system_category()),
          "socket write error");
    }
  }
  return writed;
}

}

