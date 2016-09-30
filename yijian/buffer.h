#ifndef BUFFER_H_YIJIAN
#define BUFFER_H_YIJIAN

#include <macro.h>
#include <deque>

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
    // know length
    Unwrited_Data write(const char * pos, std::size_t length) noexcept;
    // socket unknow length
    char * socket_write(int sfd);
    bool isIntegrity();

    std::size_t size();
private:
    char * header_pos_;
    char * current_pos_;
    message_type buffer_type_;
    std::size_t max_size_;
    std::size_t remain_length_;
    std::size_t integrity_size_;
};


}

#endif
