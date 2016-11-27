#include "pinglist.h"
#include <memory>

#include <stdlib.h>
#ifdef __cpluscplus
extern "C" {
#endif

PingNode * createReadPingNode() {
  YILOG_TRACE ("func : {}", __func__);
  PingNode * node = new PingNode();
  node->buffers_p.push(std::make_shared<yijian::buffer>());
  return node;
}
void destoryReadPingNode(PingNode * node) {
  YILOG_TRACE ("func : {}", __func__);
  while(!node->buffers_p.empty())
    node->buffers_p.pop();
  delete node;
}
PingNode * createWritePingNode() {
  YILOG_TRACE ("func : {}", __func__);
  PingNode * node = new PingNode();
  return node;
}
void destoryWritePingNode(PingNode * node) {
  YILOG_TRACE ("func : {}", __func__);
  delete node;
}


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

