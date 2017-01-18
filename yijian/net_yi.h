#ifndef NET_YI_H
#define NET_YI_H

#include "macro.h"
#include <leveldb/db.h>
#include <string>
#include "lib_client.h"
#include <functional>
#include <map>
#include <mutex>

class netyi {
public:
  // alias type
  typedef std::function<void(const std::string&)> CB_Func;
  // call will stop if isStop not set false
  typedef std::function<void(const std::string&, bool * isStop)> 
    CB_Func_Mutiple;

  netyi(std::string & path);
  ~netyi();
};

#endif
