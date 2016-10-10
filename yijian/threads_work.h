#ifndef THREADS_WORK_H_
#define THREADS_WORK_H_

#include "macro.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>

#ifdef __cplusplus
extern "C" {
#endif

struct PingNode;

namespace yijian {

class noti_threads : public noncopyable{
public:
  typedef std::tuple<boost::thread *, boost::mutex *,
          boost::condition_variable *> Thread_Tuple;

  noti_threads(uint_fast16_t thread_num = 4)
    : thread_count_(thread_num){
  };
  // push to vec_threads_
  void sentWork(std::function<void(void)> && func,
      std::function<void(void)> && finish);
  void foreachio(std::function<void(struct PingNode *)> && func);
private:
private:
  uint_fast16_t thread_count_;
  std::shared_ptr<std::vector<PingNode*>> vecsp_pingnode_;
  std::vector<Thread_Tuple> vec_threads_;
  boost::mutex mutex_;
  boost::condition_variable c_var_;
};


}

#ifdef __cplusplus
}
#endif

#endif
