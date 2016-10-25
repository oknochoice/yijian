#ifndef PINGLIST_H_
#define PINGLIST_H_

#include "macro.h"
#include "buffer.h"
#include <list>
#include <stdlib.h>
#include <functional>
#include <ev.h>
#include <mongocxx/cursor.hpp>
#include <queue>

#ifdef __cpluscplus
extern "C" {
#endif

union IO_Area {
  std::shared_ptr<yijian::buffer> buffer_sp;
  std::queue<std::shared_ptr<yijian::buffer>> * buffers_p;
}
struct PingNode;

typedef void* List;
typedef std::list<PingNode*> Imp_list;

struct PingNode {
  // watcher
  ev_io io;
  // if io is read, contra_io is write
  // if io is write, contra_io io read
  struct PingNode * contra_io;
  // pint list
  time_t ping_time;
  Imp_list::iterator iter;
  // socket buffer
  IO_Area io_area;
  IO_Area multimedia;
};

PingNode * createReadPingNode() {
  YILOG_TRACE ("func : {}", __func__);
  PingNode * node = malloc(sizeof(PingNode));
  node->io_area.buffer_sp = std::make_shared<yijian::buffer>();
  return node;
}
void destoryReadPingNode(PingNode * node) {
  YILOG_TRACE ("func : {}", __func__);
  free(node);
}
PingNode * createWritePingNode() {
  YILOG_TRACE ("func : {}", __func__);
  PingNode * node = malloc(sizeof(PingNode));
  node->buffer.buffers_p = new std::vector<std::shared_ptr<yijian::buffer>();
  return node;
}
void destoryWritePingNode(PingNode * node) {
  YILOG_TRACE ("func : {}", __func__);
  delete node->buffer.buffers_p;
  free(node);
}


inline List create_pinglist() {

  YILOG_TRACE ("func : {}", __func__);

  return (new Imp_list);
}

inline void ping_append(List list_sp, PingNode * p) {

  YILOG_TRACE ("func : {}", __func__);

  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  p->iter = list_sp_l->insert(list_sp_l->end(), p);
}

inline void ping_move2back(List list_sp, PingNode * p) {
  
  YILOG_TRACE ("func : {}", __func__);

  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  list_sp_l->erase(p->iter);
  p->iter = list_sp_l->insert(list_sp_l->end(), p);
}

inline void ping_erase(List list_sp, PingNode * p) {

  YILOG_TRACE ("func : {}", __func__);

  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  list_sp_l->erase(p->iter);
}

inline void ping_foreach(List list_sp, 
    std::function<void(PingNode *p, bool * isStop)> func) {

  YILOG_TRACE ("func : {}", __func__);

  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  bool is_stop_l = false;
  for (auto & node: *list_sp_l) {
    if (true == is_stop_l) {
      break;
    }
    func(node, &is_stop_l);
  }
}

#ifdef __cpluscplus
}
#endif

#endif
