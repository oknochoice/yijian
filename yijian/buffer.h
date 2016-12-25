#ifndef BUFFER_H_YIJIAN
#define BUFFER_H_YIJIAN

#include "macro.h"
#include <deque>
#include <unistd.h>
#include "typemap.h"

// 4 max var_length length 1 type length 2 sessionid length
#define SESSIONID_LENGTH 2
#define PADDING_LENGTH (1 + 4 + SESSIONID_LENGTH)

enum class Message_Type : std::size_t {
  message = 1024,
  multimedia = 1024 * 4
};

namespace yijian {

class buffer;
typedef std::shared_ptr<buffer> Buffer_SP;
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
    bool socket_read(int sfd);
    bool socket_write(int sfd);

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

    template <typename Proto> 
    void encoding(Proto && any) {

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

    template <typename Proto>
    static Buffer_SP Buffer(Proto && any);

    // session id
    uint16_t session_id();
    void set_sessionid(uint16_t sessionid);

//private:
    std::pair<uint32_t, char *>
    decoding_var_length(char * pos);
    char *
    encoding_var_length(char * pos, uint32_t length);

    std::size_t socket_read(int sfd, char * pos, std::size_t count);
    std::size_t socket_write(int sfd, char * pos, std::size_t count);
private:
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

};

template <typename Proto>
Buffer_SP buffer::Buffer(Proto && any) {

  auto buf = std::make_shared<yijian::buffer>();
  buf->encoding(std::forward<Proto>(any));
  return buf;
}

}

#endif
