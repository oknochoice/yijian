#ifndef BUFFER_H_YIJIAN
#define BUFFER_H_YIJIAN

#include "macro.h"
#include <deque>
#include <unistd.h>
#include "typemap.h"
#include "typemapre.h"
#include <google/protobuf/util/json_util.h>

#include <openssl/ssl.h>

// 4 max var_length length 1 type length 2 sessionid length
#define MSG_TYPE_LENGTH 1
#define VAR_LENGTH 2
#define SESSIONID_LENGTH 2
#define PADDING_LENGTH (MSG_TYPE_LENGTH + VAR_LENGTH + SESSIONID_LENGTH)

enum class Message_Type : std::size_t {
  message = 1024,
  multimedia = 1024 * 4
};

namespace yijian {

class buffer 
  : public yijian::noncopyable{
public:
    typedef std::pair<const char * , std::size_t> Unwrited_Data;
    
    //construct
    buffer(Message_Type type = Message_Type::message);
    void reset();
    ~buffer();
    // move constructor
    buffer(buffer&& buf) = delete;
    buffer& operator=(buffer&& buf) = delete;
    // buffer info 
    // buffer header
    char * header();
    // buffer readable length
    std::size_t size();
    // buffer writeable length
    std::size_t remain_size();
    // buffer hold length
    Message_Type buffer_type();

    // member func
    // socket read write if finish return ture
    //bool socket_read(int sfd);
    //bool socket_write(int sfd);
    bool socket_read(SSL * ssl);
    bool socket_write(SSL * ssl);

    // socket read fixed length 
    // first set length second read
    //void set_socketreadmedia_length(std::size_t length);
    //bool socket_read_media(int sfd);
    // used for proto model parse
    uint8_t datatype();
    char * data();
    std::size_t data_size();

    // encoding protocol buffer
    /*
char * buffer::data_encoding_length(uint32_t length) {
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
    */

    // session id
    uint16_t session_id();
    void set_sessionid(uint16_t sessionid);

    // c++ protobuf 
    template <typename Proto> 
    void encoding(Proto && any) {

      if (unlikely(any.ByteSize() > 1024 - PADDING_LENGTH || any.ByteSize() == 0)) {
        throw std::system_error(std::error_code(20010, std::generic_category()),
            "Malformed Length");
      }

      uint8_t type = dispatchType(any);
      current_pos_ += SESSIONID_LENGTH;
      memcpy(current_pos_, &type, 1);
      ++current_pos_;
      auto current_end = encoding_var_length(current_pos_, any.ByteSize());
      int varLength_length = current_end - current_pos_;
      current_pos_ = current_end;
      any.SerializeToArray(current_pos_, remain_size());
      remain_data_length_ = 
        SESSIONID_LENGTH + 1 + varLength_length + any.ByteSize();
      current_pos_ += any.ByteSize();
      // set buffer 
      end_pos_ = current_pos_;
      current_pos_ = header_pos_;
      data_type_ = type;
      // session_id_ send set
      YILOG_TRACE ("func: {}, type: {}, length: {}",
          __func__, type, any.ByteSize());
    }
    // c++ protobuf 
    template <typename Proto>
    static std::shared_ptr<buffer> Buffer(Proto && any);

//private:
    std::pair<uint32_t, char *>
    decoding_var_length(char * pos);
    char *
    encoding_var_length(char * pos, uint32_t length);

    std::size_t socket_read(int sfd, char * pos, std::size_t count);
    std::size_t socket_read(
        SSL * ssl, char * pos, std::size_t count);
    std::size_t socket_write(int sfd, char * pos, std::size_t count);
    std::size_t socket_write(
        SSL * ssl, char * pos, std::size_t count);
private:
    bool isParseMsgReaded_ = false;
    bool isParseFinish_ = false;
    bool isFinish_ = false;

    uint8_t data_type_;

    Message_Type buffer_type_;

    char * header_pos_;
    char * end_pos_;
    char * data_pos_;
    char * current_pos_;

    uint16_t session_id_;

    // var_length length + data_type_length 
    uint8_t parse_length_ = PADDING_LENGTH;
    // socket read or write buffer
    std::size_t remain_data_length_; 

    int_fast16_t noread_count_ = 0;
};

// c++ protobuf 
template <typename Proto>
std::shared_ptr<buffer> buffer::Buffer(Proto && any) {

  std::string value;
  google::protobuf::util::MessageToJsonString(any, &value);
  YILOG_TRACE("func: {}, any: {}", __func__, value);
  auto buf = std::make_shared<yijian::buffer>();
  buf->encoding(std::forward<Proto>(any));
  return buf;
}

}

#endif
