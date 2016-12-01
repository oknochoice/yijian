#define CATCH_CONFIG_MAIN
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "yijian/pinglist.h"
#include "yijian/lib_client.h"
#include "yijian/mongo.h"
#include "yijian/protofiles/chat_message.pb.h"
#include <google/protobuf/util/json_util.h>
#include "yijian/message_typemap.h"
#include "buffer.h"
#include <iostream>
#include <unistd.h>

#ifdef __cpluscplus
extern "C" {
#endif
// pinglist
TEST_CASE("pinglist", "[pinglist]") {
  initConsoleLog();
  List list = create_pinglist();
  PingNode* ppp[10];
  int a [10] = {0,1,2,3,4,5,6,7,8,9};
  for (int i = 0; i < 10; ++i) {
    PingNode * node = new PingNode();
    node->ping_time = a[i];
    ppp[i] = node;
    ping_append(list, node);
  }
  SECTION ("ping_append") {
    int i = 0;
    ping_foreach(list, [&](PingNode *p, bool * isStop)-> void{
          REQUIRE( static_cast<time_t>(p->ping_time) == i);
          ++i;
          *isStop = false;
        });
  }
  
  SECTION("reverse list") {
    ping_move2back(list, ppp[8]);
    ping_move2back(list, ppp[7]);
    ping_move2back(list, ppp[6]);
    ping_move2back(list, ppp[5]);
    ping_move2back(list, ppp[4]);
    ping_move2back(list, ppp[3]);
    ping_move2back(list, ppp[2]);
    ping_move2back(list, ppp[1]);
    ping_move2back(list, ppp[0]);
    int i = 9;
    ping_foreach(list, [&](PingNode *p, bool * isStop)-> void{
          REQUIRE( static_cast<time_t>(p->ping_time) == i);
          --i;
          *isStop = false;
        });
  }

  SECTION("erase") {
    for (int i = 0; i < 10; i = i + 2) {
      ping_erase(list, ppp[i]);
    }
    int i = 1;
    ping_foreach(list, [&](PingNode *p, bool * isStop)-> void{
          REQUIRE( static_cast<time_t>(p->ping_time) == i);
          i += 2;
          *isStop = false;
        });
  }
}

TEST_CASE("buffer", "[buffer]") {
  SECTION("encoding | decoing var length") {
    uint32_t length = 33;
    char data[1024];
    auto buffer_sp = new yijian::buffer();
    buffer_sp->encoding_var_length(data, length);
    auto pair = buffer_sp->decoding_var_length(data);
    REQUIRE( length == pair.first);
  }
}


TEST_CASE("IM business","[business]") {

  create_client([](Buffer_SP buffer_sp) {
      YILOG_TRACE ("client read callback");
      auto datatype = buffer_sp->datatype();
      auto datasize = buffer_sp->data_size();
      auto size = buffer_sp->size();
      YILOG_TRACE ("buffer datatype {}, datasize {}, size {}", 
          datatype, datasize, size);
      auto error = chat::Error();
      error.ParseFromArray(buffer_sp->data(), buffer_sp->data_size());
      std::string error_string;
      google::protobuf::util::MessageToJsonString(error, &error_string);
      YILOG_TRACE("error: {}", error_string);
      YILOG_TRACE("error num: {}, msg: {}", error.errnum(), error.errmsg());
      });



  sleep(1);
  SECTION("register") {
    auto regst = chat::Register();
    regst.set_phoneno("1");
    regst.set_countrycode("8");
    regst.set_password("2");
    regst.set_nickname("3");
    auto sp = encoding(regst);
    client_send(sp);
    YILOG_DEBUG ("size: {}", sp->size());
    sleep(10);
  }

}




#ifdef __cpluscplus
}
#endif

