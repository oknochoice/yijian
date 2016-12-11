#ifndef PINGLIST_H_
#define PINGLIST_H_

#include "macro.h"
#include "buffer.h"
#include <list>
#include <functional>
#include <ev.h>
#include <queue>
#include <memory>
#include <mongocxx/client.hpp>
#include "protofiles/chat_message.pb.h"

#ifdef __cpluscplus
extern "C" {
#endif

using yijian::Buffer_SP;

struct PingNode;

typedef void* List;
typedef std::list<PingNode*> Imp_list;
typedef Imp_list::iterator Iter;

struct PingNode {
  // watcher
  struct ev_io io;
  // pint list
  time_t ping_time;
  Iter iter;
};

List create_pinglist();
void ping_append(List list_sp, PingNode * p);
void ping_move2back(List list_sp, PingNode * p);
void ping_erase(List list_sp, PingNode * p);
void ping_foreach(List list_sp, 
    std::function<void(PingNode *p, bool * isStop)> func);

#ifdef __cpluscplus
}
#endif

#endif
