#define CATCH_CONFIG_RUNNER
#define YILOG_ON
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "yijian/imnet_client.h"

int main(int argc, char * argv[])
{
  const std::string raw_ip_addreess = "127.0.0.1";
  const unsigned short port_num = 5555;
  try {
    SyncTCPClient client(raw_ip_addreess, port_num);
    client.connect();
    YILOG_INFO ("Sending request to the server...");
    std::string response = client.emulateLongComputionOp(1);
    YILOG_INFO ("Response recived: {}", response);
    client.close();
  }catch (boost::system::system_error & e) {
    YILOG_ERROR ("Error occured! Error code = {}. "
        "Message: {}", e.code(), e.what());
  }
  return 0;
}
