#define CATCH_CONFIG_MAIN
#define YILOG_ON
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"


/*
int main(int argc, char * argv[])
{
  int i = 1;
  std::string s = "qqq";
  YILOG_TRACE("func :{} int: {}"
      " auok?", __func__, s );
  return 0;
}
*/

#include "yijian_test/pinglist.cpp"
