#ifndef THREADS_WORK_H_
#define THREADS_WORK_H_

#include "macro.h"

struct Read_IO;
#include <thread>
#include <condition_variable>
#include <utility>
#include <tuple>
#include <atomic>
#include <string>
#include <queue>
#include <atomic>

namespace yijian {


struct Thread_Data {
  typedef std::vector<std::shared_ptr<Read_IO>> Vector_Node;
  typedef std::shared_ptr<Vector_Node>  Vector_Node_SP;
  typedef std::function<void(void)> Thread_Function;

  std::thread thread_;

  // mainthread set subthread
  std::atomic_bool isContine_;
  // wait mainthread noti
  std::mutex c_mutex_;
  bool c_isWait_;
  std::condition_variable c_var_;

  // data in
  std::mutex q_workfun_mutex_;
  std::queue<Thread_Function> q_workfun_;
  // data out
  std::mutex v_pingnode_mutex_;
  Vector_Node_SP v_pingnode_sp_;
  
};

class noti_threads : public noncopyable{
public:

  noti_threads(uint_fast16_t thread_num = 4);
  ~noti_threads();

  // push to vec_threads_
  void sentWork(Thread_Data::Thread_Function && func);
  void foreachio(std::function<void(std::shared_ptr<Read_IO>)> && func);
  int  taskCount();
private:
  void thread_func(std::shared_ptr<Thread_Data> thread_data);
  uint_fast16_t thread_count_;
  std::vector<std::shared_ptr<Thread_Data>> vec_threads_;
  // wait subthread noti
  std::mutex c_mutex_;
  bool c_isWait_;
  std::condition_variable c_var_;
};

namespace threadCurrent {

  std::shared_ptr<Thread_Data> threadData();

  void pushPingnode(std::shared_ptr<Read_IO> node);
}


}


#endif
