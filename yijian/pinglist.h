#ifndef PINGLIST_H_
#define PINGLIST_H_

#include "macro.h"
#include "buffer.h"
#include <list>
#include <functional>
#include <ev.h>
#include <queue>
#include <memory>

#ifdef __cpluscplus
extern "C" {
#endif

class PingNode;

typedef void* List;
typedef std::list<PingNode*> Imp_list;
typedef Imp_list::iterator Iter;
typedef std::shared_ptr<yijian::buffer> Buffer_SP;

struct PingNode {
  // watcher
  ev_io io;
  // if io is read, contra_io is write
  // if io is write, contra_io io read
  struct PingNode * contra_io;
  // pint list
  time_t ping_time;
  Iter iter;
  // socket buffer
  std::queue<Buffer_SP> buffers_p;
};


PingNode * createReadPingNode();
void destoryReadPingNode(PingNode * node);
PingNode * createWritePingNode();
void destoryWritePingNode(PingNode * node);


inline List create_pinglist();

inline void ping_append(List list_sp, PingNode * p);

inline void ping_move2back(List list_sp, PingNode * p);

inline void ping_erase(List list_sp, PingNode * p);

inline void ping_foreach(List list_sp, 
    std::function<void(PingNode *p, bool * isStop)> func);

#ifdef __cpluscplus
}
#endif

#endif
