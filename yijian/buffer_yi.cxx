#include "buffer_yi.h"

#include "macro.h"
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <bitset>

#include <openssl/err.h>

// uint_16: 1 << 15
#define BufferLastFlag (32768)

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


bool buffer::socket_read(SSL * sfd) {
  YILOG_TRACE("func: {}", __func__);
  // parse length and data type
  YILOG_TRACE("func: {}, isParseFinish_: {}, isFinish_: {}", 
      __func__, isParseFinish_, isFinish_);
  bool isContine = false;
  do {
  isContine = false;

  try {

  if (!isParseFinish_) {
    if (false == isParseMsgReaded_) {
      YILOG_TRACE ("func: {}, read header", __func__);
      int readed = socket_read(sfd, current_pos_, parse_length_);
      current_pos_ += readed;
      parse_length_ -= readed;
      int loopCount = current_pos_ - header_pos_ - SESSIONID_LENGTH - MSG_TYPE_LENGTH;
      if (loopCount > 0) {
        for (int i = 0; i < loopCount; ++i) {
          uint8_t checkbit = *(header_pos_ + SESSIONID_LENGTH + VAR_LENGTH + i);
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
      uint16_t session_net = *reinterpret_cast<uint16_t*>(header_pos_);
      YILOG_TRACE ("session_net: {}", std::bitset<16>(session_net).to_string());
      auto sessionid = ntohs(session_net);
      YILOG_TRACE ("session_loc: {}", std::bitset<16>(sessionid).to_string());
      if (sessionid >= BufferLastFlag) {
        isLastBuf_ = true;
        session_id_ = sessionid - BufferLastFlag;
      }else {
        isLastBuf_ = false;
        session_id_ = sessionid;
      }
      YILOG_TRACE ("func: {}, buf_sessionid: {}, isLastBuf: {}, sessionid: {}", 
          __func__, sessionid, isLastBuf_, session_id_);
      
      // type
      data_type_ =  *(header_pos_ + SESSIONID_LENGTH);
      // var_length
      auto pair = decoding_var_length(header_pos_ + SESSIONID_LENGTH + MSG_TYPE_LENGTH);
      data_length_ = pair.first;
      data_encode_length_ = pair.second - data_pos_;
      data_pos_ = pair.second;

      YILOG_TRACE ("func: {}, type: {}, length: {}", 
          __func__, data_type_, pair.first);

      long readed = PADDING_LENGTH - (pair.second - header_pos_);

      remain_data_length_ = pair.first - readed;
      isParseFinish_ = true;

      if (remain_data_length_ + PADDING_LENGTH > 
          static_cast<std::size_t>(buffer_type_)) {
        throw std::system_error(std::error_code(20000, std::generic_category()),
            "transfer data over buffer size");
      }
      isContine = true;
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

  }catch (std::system_error & e) {
    throw;
  }

  }while(isContine && !isFinish_);

  YILOG_TRACE("func: {} read finish {}", __func__, isFinish_);
  return isFinish_;
}

bool buffer::socket_write(SSL * sfd) {
  YILOG_TRACE("func: 1 argm {}", __func__);
  YILOG_TRACE("func: 1 argm {}, remain length{}", __func__, remain_data_length_);
  remain_data_length_ -= socket_write(sfd, current_pos_, remain_data_length_);
  YILOG_TRACE("func: 1 argm {}, remain length{}", __func__, remain_data_length_);
  if (0 == remain_data_length_) {
    isFinish_ = true;
    //BIO * wbio = SSL_get_wbio(sfd);
    //BIO_flush(wbio);
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
  //return end_pos_ - data_pos_;
  return data_length_;
}


uint16_t buffer::session_id() {
  return session_id_;
}
void buffer::set_sessionid(const uint16_t sessionid, const bool isLast) {
  YILOG_TRACE("func: {}", __func__);
  if (unlikely(sessionid > MaxSessionID)) {
    YILOG_ERROR("session id over flow");
  }
  uint16_t loc_sessionid = sessionid;
  isLastBuf_ = isLast;
  if (unlikely(isLast)) {
    loc_sessionid += BufferLastFlag;
  }
  uint16_t sessionid_l= htons(loc_sessionid);
  memcpy(header_pos_, &sessionid_l, 2);
  session_id_ = sessionid;
}
bool buffer::isLast_buffer() {
  return isLastBuf_;
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

std::size_t buffer::socket_read(SSL * ssl, char * pos, std::size_t count) {
  YILOG_TRACE("func: {}", __func__);
  int readed = SSL_read(ssl, pos, count);
  if (0 < readed) {
    YILOG_TRACE("func: {}, readed: {}", __func__, readed);
    noread_count_ = 0;
  }else {
    int error_l = SSL_get_error(ssl, readed);
    if ( error_l == SSL_ERROR_ZERO_RETURN) {
      YILOG_ERROR("func: {}, SSL_ERROR_ZERO_RETURN:"
          "connection has been closed", __func__);
      throw std::system_error(std::error_code(
            20005, std::system_category()),
          "connection has been closed");
    }else if (error_l == SSL_ERROR_WANT_READ) {
      YILOG_TRACE("func: {}, SSL_ERROR_WANT_READ:", __func__);
      throw std::system_error(std::error_code(
            20012, std::system_category()),
          "read later");
    }else if (error_l == SSL_ERROR_WANT_CONNECT || 
        error_l == SSL_ERROR_WANT_ACCEPT) {
      YILOG_TRACE("func: {}, SSL_ERROR_WANT_CONNECT"
          " SSL_ERROR_WANT_ACCEPT:", __func__);
      readed = 0;
    }else if (error_l == SSL_ERROR_WANT_X509_LOOKUP ){
      YILOG_ERROR("func: {}, SSL_ERROR_WANT_X509_LOOKUP"
          , __func__);
      throw std::system_error(std::error_code(
            20006, std::system_category()),
          "SSL_ERROR_WANT_X509_LOOKUP");
    }else if (error_l == SSL_ERROR_SYSCALL ){
      YILOG_ERROR("func: {}, errno: {}", __func__, errno);
      perror("ssl_read SSL_ERROR_SYSCALL");
      throw std::system_error(std::error_code(
            20007, std::system_category()),
          "SSL_ERROR_SYSCALL");
    }else {
      YILOG_ERROR("func: {}, unknow error", __func__);
      throw std::system_error(std::error_code(
            20008, std::system_category()),
          "unknow error");
    }
  }
  return readed;
}

std::size_t buffer::socket_write(SSL * ssl, char * pos, std::size_t count) {
  YILOG_TRACE("func: 3 argm{}", __func__);
  int writed = SSL_write(ssl, pos, count);
  if (0 < writed) {
    YILOG_TRACE("func: {}, writed {}", __func__, writed);
  }else {
    int error_l = SSL_get_error(ssl, writed);
    if ( error_l == SSL_ERROR_ZERO_RETURN) {
      YILOG_ERROR("func: {}, SSL_ERROR_ZERO_RETURN:"
          "connection has been closed", __func__);
      throw std::system_error(std::error_code(
            20009, std::system_category()),
          "connection has been closed");
    }else if (error_l == SSL_ERROR_WANT_WRITE) {
      YILOG_TRACE("func: {}, SSL_ERROR_WANT_READ:", __func__);
      throw std::system_error(std::error_code(
            20024, std::system_category()),
          "write again now");
    }else if (error_l == SSL_ERROR_WANT_CONNECT || 
        error_l == SSL_ERROR_WANT_ACCEPT) {
      YILOG_TRACE("func: {}, SSL_ERROR_WANT_CONNECT"
          " SSL_ERROR_WANT_ACCEPT:", __func__);
      writed = 0;
    }else if (error_l == SSL_ERROR_WANT_X509_LOOKUP ){
      YILOG_ERROR("func: {}, SSL_ERROR_WANT_X509_LOOKUP"
          , __func__);
      throw std::system_error(std::error_code(
            20020, std::system_category()),
          "SSL_ERROR_WANT_X509_LOOKUP");
    }else if (error_l == SSL_ERROR_SYSCALL ){
      YILOG_ERROR("func: {}, errno: {}", __func__, errno);
      throw std::system_error(std::error_code(
            20022, std::system_category()),
          "SSL_ERROR_SYSCALL no: " + std::to_string(errno));
    }else {
      YILOG_ERROR("func: {}, unknow error", __func__);
      ERR_print_errors_fp(stderr);
      throw std::system_error(std::error_code(
            20023, std::system_category()),
          "other error no: " + std::to_string(error_l));
    }
  }
  return writed;
}
void buffer::makeReWrite() {
  current_pos_ = header_pos_;
  remain_data_length_ =
    SESSIONID_LENGTH + 1 + data_encode_length_ + data_length_;
}
  
void buffer::encoding(const uint8_t type, const std::string & data) {
  data_length_ = data.length();
  Assert(data_length_ > 0);
  Assert(data_length_ <= 1024 - PADDING_LENGTH);
  if (unlikely(data_length_ > 1024 - PADDING_LENGTH || data_length_ == 0)) {
    throw std::system_error(std::error_code(20011, std::generic_category()),
        "Malformed Length");
  }

  current_pos_ += SESSIONID_LENGTH;
  memcpy(current_pos_, &type, 1);
  ++current_pos_;
  auto current_end = encoding_var_length(current_pos_, data_length_);
  data_encode_length_ = current_end - current_pos_;
  data_pos_ = current_pos_ = current_end;
  memcpy(current_pos_, data.c_str(), data_length_);
  remain_data_length_ =
    SESSIONID_LENGTH + 1 + data_encode_length_ + data_length_;
  current_pos_ += data_length_;
  // set buffer 
  end_pos_ = current_pos_;
  current_pos_ = header_pos_;
  data_type_ = type;
  // session_id_ send set
  YILOG_TRACE ("func: {}, type: {}, length: {}",
      __func__, type, data_length_);
}
  
std::shared_ptr<buffer> buffer::Buffer(const uint8_t type, const std::string & data) {
  auto buf = std::make_shared<yijian::buffer>();
  buf->encoding(type, data);
  return buf;
}
  
}

