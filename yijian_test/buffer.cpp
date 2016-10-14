#include <../yijian/buffer.h>

#ifdef __cpluscplus
extern "C" {
#endif


TEST_CASE ("buffer ", "[buffer]") {


  SECTION ("ping_append") {
    int i = 0;
    ping_foreach(list, [&](PingNode *p, bool * isStop)-> void{
          REQUIRE( static_cast<time_t>(p->ping_time) == i);
          ++i;
          *isStop = false;
        });
  }
}

#ifdef __cpluscplus
}
#endif
