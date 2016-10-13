#ifndef THREADS_WORK_H_
#define THREADS_WORK_H_

#include "macro.h"

struct PingNode;
#include <thread>
#include <condition_variable>
#include <utility>
#include <tuple>
#include <atomic>
#include <string>

namespace yijian {

struct Thread_Data {
  typedef std::vector<PingNode*> Vector_Node;
  typedef std::shared_ptr<Vector_Node>  Vector_Node_SP;

  std::thread thread_;

  std::atomic_bool isContine_;
  std::mutex c_mutex_;
  bool c_isWait_;
  std::condition_variable c_var_;

  std::function<void(void)> && work_func_;
  std::function<void(void)> && workFinish_func_;

  std::mutex v_mutex_;
  Vector_Node_SP v_pingnode_sp_;
  PingNode * current_node_;
};

class noti_threads : public noncopyable{
public:

  noti_threads(uint_fast16_t thread_num = 4);
  ~noti_threads();

  // push to vec_threads_
  void sentWork(std::function<void(void)> && func,
      PingNode * node, 
      std::function<void(void)> && finish);
  void foreachio(std::function<void(struct PingNode *)> && func);
  void thread_func(Thread_Data * thread_data);
private:
  uint_fast16_t thread_count_;
  std::vector<Thread_Data*> vec_threads_;
  std::mutex c_mutex_;
  bool c_isWait_;
  std::condition_variable c_var_;
};


}


#endif
