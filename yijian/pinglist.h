#ifndef PINGLIST_H_
#define PINGLIST_H_

#include "macro.h"
#include <list>
#include <stdlib.h>
#include <functional>
#include <ev.h>

#ifdef __cpluscplus
extern "C" {
#endif

struct PingNode;

typedef void* List;
typedef std::list<PingNode*> Imp_list;

struct PingNode {
  ev_io io;
  time_t ping_time;
  Imp_list::iterator iter;
};


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
