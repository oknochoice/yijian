#include <../yijian/pinglist.h>

#ifdef __cpluscplus
extern "C" {
#endif


TEST_CASE("pinglist", "[pinglist]") {
  List list = create_pinglist();
  int i0 = 0;
  int i1 = 1;
  int i2 = 2;
  int i3 = 3;
  int i4 = 4;
  int i5 = 5;
  int i6 = 6;
  int i7 = 7;
  int i8 = 8;
  int i9 = 9;
  ping_append(list, &i0);
  ping_append(list, &i1);
  ping_append(list, &i2);
  ping_append(list, &i3);
  ping_append(list, &i4);
  ping_append(list, &i5);
  ping_append(list, &i6);
  ping_append(list, &i7);
  ping_append(list, &i8);
  ping_append(list, &i9);
  SECTION ("ping_append") {
    int i = 0;
    ping_foreach(list, [&](void *p, bool * isStop)-> void{
          REQUIRE( *static_cast<int*>(p) == i);
          ++i;
          *isStop = false;
        });
  }
  
  SECTION("reverse list") {
    ping_move2back(list, &i8);
    ping_move2back(list, &i7);
    ping_move2back(list, &i6);
    ping_move2back(list, &i5);
    ping_move2back(list, &i4);
    ping_move2back(list, &i3);
    ping_move2back(list, &i2);
    ping_move2back(list, &i1);
    ping_move2back(list, &i0);
    int i = 9;
    ping_foreach(list, [&](void *p, bool * isStop)-> void{
          REQUIRE( *static_cast<int*>(p) == i);
          --i;
          *isStop = false;
        });
  }
}

#ifdef __cpluscplus
}
#endif
