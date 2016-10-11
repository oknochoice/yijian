#ifndef THREADS_WORK_H_
#define THREADS_WORK_H_

#include "macro.h"

struct PingNode;
#include <thread>
#include <condition_variable>
#include <utility>

namespace yijian {

class noti_threads : public noncopyable{
public:
  typedef std::pair<std::mutex *, 
            std::condition_variable *>  Mutex_C_Var;
  typedef std::pair<std::thread *, 
            Mutex_C_Var * > Thread_Data;

  noti_threads(uint_fast16_t thread_num = 4);
  ~noti_threads();
  // push to vec_threads_
  void sentWork(std::function<void(void)> && func,
      std::function<void(void)> && finish);
  void foreachio(std::function<void(struct PingNode *)> && func);
  static void thread_func(Mutex_C_Var * mutex_c_var);
private:
  uint_fast16_t thread_count_;
  std::shared_ptr<std::vector<PingNode*>> vecsp_pingnode_;
  std::vector<Thread_Data*> vec_threads_;
  std::mutex mutex_;
  std::condition_variable c_var_;
};


}


#endif
