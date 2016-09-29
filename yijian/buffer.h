#ifndef BUFFER_H_YIJIAN
#define BUFFER_H_YIJIAN

#include <macro.h>
#include <deque>

#define BUFFER_SIZE 

namespace yijian {


class buffer 
  : public yijian::noncopyable{
public:
    typedef std::pair<const uint8_t * , uint_fast32_t> Unwrited_Data;
    
    //construct
    buffer();
    // move constructor
    buffer(buffer&& buf) = delete;
    buffer& operator=(buffer&& buf) = delete;

    ~buffer();
    // member func
    bool setIntegritySize(uint_fast64_t size);
    bool isIntegrity();
    Unwrited_Data write(const uint8_t * pos, std::size_t length) noexcept;
    std::size_t size();
    std::size_t writable_size();
private:
    uint8_t * header_pos_;
    uint8_t * current_pos_;
    std::size_t max_size_;
    std::size_t remain_length_;
    std::size_t integrity_size_;
};


}

#endif
