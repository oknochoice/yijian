#ifndef BUFFER_H_YIJIAN
#define BUFFER_H_YIJIAN

#define MinSessionID 100
#define MaxSessionID 32767

#include "macro.h"
#include <deque>
#include <unistd.h>
#include "typemap.h"

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
    void makeReWrite();

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
    void set_sessionid(const uint16_t sessionid, const bool isLast);
    bool isLast_buffer();
    
    void encoding(const uint8_t type, const std::string & data);
    static std::shared_ptr<buffer> Buffer(const uint8_t type, const std::string & data);
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
    bool isLastBuf_ = false;

    uint8_t data_type_;
    std::size_t data_encode_length_;
    std::size_t data_length_;

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


}

#endif
