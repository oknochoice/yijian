#include "pinglist.h"
#include <memory>

#include <stdlib.h>
#ifdef __cpluscplus
extern "C" {
#endif

List create_pinglist() {

  YILOG_TRACE ("func : {}", __func__);

  return (new Imp_list);
}

void ping_append(List list_sp, PingNode * p) {

  YILOG_TRACE ("func : {}", __func__);

  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  p->iter = list_sp_l->insert(list_sp_l->end(), p);
}

void ping_move2back(List list_sp, PingNode * p) {
  
  YILOG_TRACE ("func : {}", __func__);

  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  list_sp_l->erase(p->iter);
  p->iter = list_sp_l->insert(list_sp_l->end(), p);
}

void ping_erase(List list_sp, PingNode * p) {

  YILOG_TRACE ("func : {}", __func__);

  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  list_sp_l->erase(p->iter);
}

void ping_foreach(List list_sp, 
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

