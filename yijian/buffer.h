#ifndef __BUFFER_H__
#define __BUFFER_H__
#include <macro.h>
#include <deque>

namespace yijian {


enum class message_type : uint_fast64_t{
  fixed_header = 5,
  variable_header = 10,
  small_message = 128,
  message = 1024,
  image = 1024 * 1024 * 256,
  big_image = 1024 * 1024 * 1024
};

class buffer_loop
  : public yijian::noncopyable {
public:
    //construct
    buffer_loop(uint_fast64_t buffer_size, 
        uint_fast32_t buffer_count) ;
    // member func
    uint8_t* get();
    void put(uint8_t *);
private:
    void alloc(uint_fast64_t buffer_size, 
              uint_fast32_t buffer_count);
private:
    std::deque<uint8_t *> deque_;
    uint_fast64_t buffer_size_;
    uint_fast32_t buffer_count_;
    uint_fast32_t block_count_ = 0;
};


static buffer_loop& get_loop(message_type msg_type);

class buffer 
  : public yijian::noncopyable{
public:
    typedef std::pair<const uint8_t * , uint_fast32_t> Unwrited_Data;
    
    //construct
    buffer(message_type msg_type);
    // move constructor
    buffer(buffer&& buf) noexcept;
    buffer& operator=(buffer&& buf) noexcept;

    ~buffer() {clean();}
    // member func
    Unwrited_Data write(const uint8_t * pos, uint_fast64_t length) noexcept;
    void write(uint8_t value);
    void write(uint16_t value);
    void write(std::string && str);
    std::size_t size();
private:
    void clean();
private:
    uint8_t * header_pos_;
    uint8_t * current_pos_;
    uint_fast64_t max_size_;
    uint_fast64_t remain_length_;
    message_type msg_type_;
    bool isNeedRelease_;
};


class buffers 
  : public yijian::noncopyable{
public:
    //construct
    buffers(message_type msg_type);
    //member func
    void write(uint8_t * pos, uint_fast64_t length);
private:
private:
    std::vector<buffer> array_;
    uint8_t * pos_;
    message_type msg_type_;
};

}

#endif
