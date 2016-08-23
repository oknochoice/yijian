#include <macro.h>
#include <deque>

namespace yijian {



class buffer 
  : public yijian::noncopyable{
    typedef std::pair<char * , uint_fast32_t> Unwrited_Data;
public:
    buffer(char * header, uint_fast32_t length) 
      : header_pos_(header), current_pos_(header),
      max_size_(length), remain_length_(length){
    };
    Unwrited_Data write(char * pos, uint_fast32_t length) noexcept;
    char * header() {
      return header_pos_;
    }
    std::size_t size() {
      return current_pos_ - header_pos_;
    }
private:
    char * header_pos_;
    char * current_pos_;
    uint_fast32_t max_size_;
    uint_fast32_t remain_length_;
};

class buffer_loop
  : public yijian::noncopyable {
public:
    buffer_loop(uint_fast64_t buffer_size, 
        uint_fast32_t buffer_count) 
      : deque_(std::deque<char*>(buffer_count)), 
        buffer_size_(buffer_size) {
      alloc();
    }
    buffer get() {
      if (!deque_.empty()) {
        if( 5 == block_count_ ) {
          YILOG_WARN("buffer loop's blocks too many to alloc "
                    "maybe not malloc");
        }
        alloc();
      }
      char * p = deque_.front();
      deque_.pop_front();
      return buffer(p, buffer_size_);
    }

    void put(buffer && buf) {
      deque_.push_back(buf.header());
    }
private:
    void alloc() {
      char * p = (char *)std::malloc(buffer_size_ * buffer_count_);
      int_fast32_t i = -1;
      std::generate_n(std::back_insert_iterator<std::deque<char*> >(deque_),
                      buffer_count_, [=]() mutable {++i; 
                                        return p + i * buffer_size_;});
      ++block_count_;
    }
    std::deque<char *> deque_;
    uint_fast64_t buffer_size_;
    uint_fast32_t buffer_count_;
    uint_fast32_t block_count_ = 0;
};

enum class message_type : uint_fast64_t{
  fixed_header = 5,
  variable_header = 10,
  small_message = 128,
  message = 1024,
  image = 1024 * 1024 * 256,
  big_image = 1024 * 1024 * 1024
};

static buffer_loop& get_loop(message_type msg_type) {
  switch(msg_type) {
    case message_type::fixed_header: 
      static thread_local buffer_loop loop1 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop1;
    case message_type::variable_header: 
      static thread_local buffer_loop loop2 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop2;
    case message_type::small_message: 
      static thread_local buffer_loop loop3 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop3;
    case message_type::message: 
      static thread_local buffer_loop loop4 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop4;
    case message_type::image: 
      static thread_local buffer_loop loop5 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop5;
    case message_type::big_image: 
      static thread_local buffer_loop loop6 = 
      buffer_loop(static_cast<uint_fast64_t>(msg_type), MQTT_SESSION_COUNT);
      return loop6;
  }
}

class buffers 
  : public yijian::noncopyable{
public:
    buffers(message_type msg_type): array_(std::vector<buffer>(5)), 
        m_type_(msg_type) {
      array_.push_back(get());
    };
    void write(char * pos, uint_fast32_t length);
private:
    buffer get() {
      return get_loop(m_type_).get();
    };
    std::vector<buffer> array_;
    char * pos_;
    message_type m_type_;
};

buffer::Unwrited_Data buffer::write(char * pos, uint_fast32_t length) noexcept{
  uint_fast32_t real_length;
  if (remain_length_ >= length) {
    real_length = length;
  }else {
    real_length = remain_length_;
  }
  std::memcpy(current_pos_, pos, real_length);
  current_pos_ += real_length;
  remain_length_ -= length;
  if (0 == remain_length_) {
    return std::make_pair(pos + real_length, length - real_length);
  }else {
    return std::make_pair(nullptr, 0);
  }
}

void buffers::write(char * pos, uint_fast32_t length) {
  auto * buf = &array_.back();
  char * current_pos_ = pos;
  do {
    auto pair = buf->write(current_pos_, length);
    if (0 != pair.second) {
      array_.push_back(get());
      buf = &array_.back();
      current_pos_ = pair.first;
    }else {
      break;
    }
  }while (true);
}

}

