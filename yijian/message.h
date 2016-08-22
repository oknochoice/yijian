#include "macro.h"

class packet
  : public yijian::noncopyable {
public:
  typedef std::pair<std::shared_ptr<char *>, int> Data;
  typedef std::vector<Data> Payload;
private:
private:
  Data fixed_header_;
  Data variable_header_;
  Payload payload_;
};

class connect
  : public packet {
public:
  connect() {
    YILOG_TRACE(__func__);
  }
};
