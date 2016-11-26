#define CATCH_CONFIG_MAIN
#define YILOG_ON
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "yijian/pinglist.h"
#include "yijian/lib_client.h"
#include "yijian/mongo.h"
#include "yijian/protofiles/chat_message.pb.h"
#include "yijian/message_typemap.h"

#ifdef __cpluscplus
extern "C" {
#endif
// pinglist
TEST_CASE("pinglist", "[pinglist]") {
  List list = create_pinglist();
  PingNode* ppp[10];
  int a [10] = {0,1,2,3,4,5,6,7,8,9};
  for (int i = 0; i < 10; ++i) {
    PingNode * node = static_cast<PingNode*>(malloc(sizeof(PingNode)));
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


TEST_CASE("IM business","[business]") {

  create_client([](Buffer_SP buffer_sp) {
      std::cout << "client read callback" << std::endl;
      });



  SECTION("register") {
    auto regst = chat::Register();
    regst.set_phoneno("18514029918");
    regst.set_countrycode("86");
    regst.set_password("123456");
    regst.set_nickname("yijian");
    auto sp = encoding(regst);
    client_send(sp);


  }

}




#ifdef __cpluscplus
}
#endif

