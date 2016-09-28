#ifndef PINGLIST_H_
#define PINGLIST_H_

#include "macro.h"
#include <list>
#include <stdlib.h>
#include <functional>

#ifdef __cpluscplus
extern "C" {
#endif

typedef void* List;
typedef std::list<void*> Imp_list;

struct Node {
  void * data;
  Imp_list::iterator iter;
};


inline List create_pinglist() {

  YILOG_TRACE ("func :", __func__);

  return (new Imp_list);
}

inline void ping_append(List list_sp, Node * p) {

  YILOG_TRACE ("func :", __func__);

  struct Node * node = 
    reinterpret_cast<struct Node*>(malloc(sizeof(struct Node)));
  node->data = p;
  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  node->iter = list_sp_l->insert(list_sp_l->end(), node);
}

inline void ping_move2back(List list_sp, Node * p) {
  
  YILOG_TRACE ("func :", __func__);

  struct Node * node = reinterpret_cast<struct Node *>(p);
  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  list_sp_l->erase(node->iter);
  node->iter = list_sp_l->insert(list_sp_l->end(), node);
}

inline void ping_erase(List list_sp, Node * p) {

  YILOG_TRACE ("func :", __func__);

  struct Node * node = reinterpret_cast<struct Node *>(p);
  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  list_sp_l->erase(node->iter);
  free(node);
}

inline void ping_foreach(List list_sp, 
    std::function<void(Node *p, bool * isStop)> func) {

  YILOG_TRACE ("func :", __func__);

  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  bool is_stop_l = false;
  for (auto & node: *list_sp_l) {
    if (true == is_stop_l) {
      break;
    }
    struct Node * n = reinterpret_cast<struct Node*>(&node);
    func(n->data, &is_stop_l);
  }
}

#ifdef __cpluscplus
}
#endif

#endif
