#include "yijian/libev_server.h"
#include "macro.h"
#include <vector>
#include <iostream>

int main(int argc, char * argv[]) {
  initConsoleLog();
  YILOG_TRACE ("func: {}. ", __func__);
  auto ips = std::vector<std::pair<std::string, int>>();
  return start_server_libev();
}
