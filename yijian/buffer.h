#ifndef BUFFER_H_YIJIAN
#define BUFFER_H_YIJIAN

#include "macro.h"
#include <deque>
#include <unistd.h>

#define MESSAGE_TYPE_LENGTH 1

enum class Message_Type : std::size_t {
  message = 128,
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
    bool socket_read(int sfd);
    bool socket_write(int sfd);
    // socket read fixed length 
    // first set length second read
    void set_socketreadmedia_length(std::size_t length);
    bool socket_read_media(int sfd);
    // used for proto model parse
    uint_fast8_t datatype();
    char * data();
    std::size_t data_size();
    // encoding proto model to buffer
    void data_encoding_length(uint_fast32_t length);
    void data_encoding_type(uint8_t type);
    char * data_encoding_current();
    void data_encoding_reset_size(std::size_t length);
//private:
    std::pair<uint_fast32_t, char *>
    decoding_var_Length(char * pos);
    char *
    encoding_var_Length(char * pos, uint_fast32_t length);

    std::size_t socket_read(int sfd, std::size_t count);
    std::size_t socket_write(int sfd, std::size_t count);
private:
    bool isParseFinish_ = false;
    bool isFinish_ = false;

    uint_fast8_t data_type_;

    Message_Type buffer_type_;

    char * header_pos_;
    char * data_pos_;
    char * current_pos_;

    // var_length length + data_type_length 
    uint_fast8_t parse_length_ = 4 + MESSAGE_TYPE_LENGTH;
    // socket read or write buffer
    std::size_t remain_data_length_; 

};


}

#endif
