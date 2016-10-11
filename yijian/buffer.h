#ifndef BUFFER_H_YIJIAN
#define BUFFER_H_YIJIAN

#include <macro.h>
#include <deque>
#include <unistd.h>

#define MESSAGE_TYPE_LENGTH 1

enum class message_type : std::size_t {
  message = 128,
  multimedia = 1024 * 4
};

namespace yijian {

class buffer 
  : public yijian::noncopyable{
public:
    typedef std::pair<const char * , std::size_t> Unwrited_Data;
    
    //construct
    buffer(message_type type);
    // move constructor
    buffer(buffer&& buf) = delete;
    buffer& operator=(buffer&& buf) = delete;

    ~buffer();
    // member func
    // socket read write if finish return ture
    bool socket_read(int sfd);
    bool socket_write(int sfd);

    uint_fast16_t datatype();
    char * data();
    std::size_t data_size();
    
    char * header();
    std::size_t size();

private:
    std::pair<uint_fast32_t, char *>
    decoding_var_Length(char * pos);
    char *
    encoding_var_Length(char * pos, uint_fast32_t length);

    std::size_t socket_read(int sfd, std::size_t count);
    std::size_t socket_write(int sfd, std::size_t count);
private:
    bool isParseFinish_ = false;
    bool isFinish_ = false;

    uint_fast16_t data_type_;

    message_type buffer_type_;

    char * header_pos_;
    char * data_pos_;
    char * current_pos_;

    // var_length length + data_type_length 
    uint_fast8_t parse_length_ = 4 + MESSAGE_TYPE_LENGTH;
    std::size_t remain_data_length_;

};


}

#endif
