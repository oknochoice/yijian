#define CATCH_CONFIG_RUNNER
#define YILOG_ON
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"


int main(int argc, char * argv[])
{
  YILOG_TRACE("func :{}", __func__);
  return 0;
}

