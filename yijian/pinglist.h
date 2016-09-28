#ifndef PINGLIST_H_
#define PINGLIST_H_

#include "macro.h"
#include <list>
#include <stdlib.h>

#ifdef __cpluscplus
extern "C" {
#endif

typedef void* List;
typedef std::list<void*> Imp_list;

struct Node {
  void * data;
  Imp_list::iterator iter;
};


inline List creat_pinglist() {

  YILOG_TRACE ("func :", __func__);

  return (new Imp_list);
}

inline void append(List list_sp, void * p) {

  YILOG_TRACE ("func :", __func__);

  struct Node * node = 
    reinterpret_cast<struct Node*>(malloc(sizeof(struct Node)));
  node->data = p;
  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  node->iter = list_sp_l->insert(list_sp_l->end(), node);
}

inline void move2back(List list_sp, void * p) {
  
  YILOG_TRACE ("func :", __func__);

  struct Node * node = reinterpret_cast<struct Node *>(p);
  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  list_sp_l->erase(node->iter);
  node->iter = list_sp_l->insert(list_sp_l->end(), node);
}

inline void erase(List list_sp, void * p) {

  YILOG_TRACE ("func :", __func__);

  struct Node * node = reinterpret_cast<struct Node *>(p);
  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  list_sp_l->erase(node->iter);
  free(node);
}

inline void foreach_list(List list_sp, 
    void *func(void *p, bool * isStop)) {

  YILOG_TRACE ("func :", __func__);

  auto list_sp_l = reinterpret_cast<Imp_list*>(list_sp);
  bool is_stop_l = false;
  for (auto & node: *list_sp_l) {
    if (true == is_stop_l) {
      break;
    }
    func(&node, &is_stop_l);
  }
}

#ifdef __cpluscplus
}
#endif

#endif
