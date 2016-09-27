#define CATCH_CONFIG_RUNNER
#define YILOG_ON

#include "macro.h"
#include "catch.hpp"
#include "yijian/imnet.h"


int main (int argc, char * argv[])
{
  unsigned short port_num = 5555;
  try {
    Server srv;
    srv.Start (port_num);
  }catch (boost::system::system_error & e) {
    std::ostringstream code;
    code << e.code();
    YILOG_ERROR ("error code = {}, message: {}",
        code.rdstate(), e.what());
  }
  return 0;
}
