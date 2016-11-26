#include "threads_work.h"
#include "stdlib.h"

namespace yijian {

// noti threads
noti_threads::noti_threads(uint_fast16_t thread_num)
  : thread_count_(thread_num){
  YILOG_TRACE("func: {}", __func__);
    for (uint_fast16_t i = 0; i < thread_num; i++) {
      auto thread_data = (Thread_Data*)malloc(sizeof(Thread_Data));
      thread_data->thread_ = 
//        std::thread(&noti_threads::thread_func, thread_data);
          std::thread([&]() {
               this->thread_func(thread_data);
              });
      thread_data->isContine_ = true;
      thread_data->c_isWait_ = true;
      thread_data->v_pingnode_sp_.reset(new Thread_Data::Vector_Node);
      vec_threads_.push_back(thread_data);
    }
};

noti_threads::~noti_threads() {
  YILOG_TRACE("func: {}", __func__);
  for(auto thread_data: vec_threads_) {
    thread_data->thread_.join();
    free(thread_data);
  }
}

void noti_threads::sentWork(Thread_Data::Thread_Function && func) {

  YILOG_TRACE("func: {}", __func__);

  bool isContine = true;

  while(isContine) {

    Thread_Data * thread_local_data = nullptr;
    for (auto thread_data: vec_threads_) {
      std::unique_lock<std::mutex> ul(thread_data->q_workfun_mutex_);
      if (thread_local_data == nullptr) {
        thread_local_data = thread_data;
        continue;
      }
      if (thread_local_data->q_workfun_.size() > 
          thread_data->q_workfun_.size()) {
        thread_local_data = thread_data;
      }
    }

    {
      std::unique_lock<std::mutex> ul(thread_local_data->q_workfun_mutex_);
      if (thread_local_data->q_workfun_.size() < 20) {

        thread_local_data->q_workfun_.push(
            std::forward<Thread_Data::Thread_Function>(func)
            );

        isContine = false;
      }
    }
      
    if (isContine) {
      std::unique_lock<std::mutex> ul(c_mutex_);
      c_var_.wait(ul, [&](){
          return !c_isWait_;
          });
      c_isWait_ = true;
    }else {
      std::unique_lock<std::mutex> ul(thread_local_data->c_mutex_);
      thread_local_data->c_isWait_ = false;
      thread_local_data->c_var_.notify_one();
    }


  }
}

void noti_threads::foreachio(
    std::function<void(struct PingNode *)> && func) {

  for (auto thread_data: vec_threads_) {
    Thread_Data::Vector_Node_SP data_sp;
    {
      std::unique_lock<std::mutex> ul(thread_data->v_pingnode_mutex_);
      data_sp = thread_data->v_pingnode_sp_;
    }
    for (auto node: *data_sp) {
      func(node);
    }
  }

}

namespace threadCurrent {

Thread_Data * threadData(Thread_Data* currentData) {
  YILOG_TRACE("func: {}", __func__);
  static thread_local auto threaddata = currentData;
  return threaddata;
}

Thread_Data * threadData() {
  YILOG_TRACE("func: {}", __func__);
  return threadData(nullptr);
}

void pushPingnode(PingNode * node) {
  YILOG_TRACE("func: {}", __func__);

  auto thread_data = threadData();
  std::unique_lock<std::mutex> ul(thread_data->v_pingnode_mutex_);
  auto v_pingnode = thread_data->v_pingnode_sp_;
  if (!v_pingnode.unique()) {
    v_pingnode.reset(new Thread_Data::Vector_Node);
  }
  v_pingnode->push_back(node);

}

}

void noti_threads::thread_func(Thread_Data* thread_data) {

  YILOG_TRACE("func: {}", __func__);
  threadCurrent::threadData(thread_data);
  
  while (true) {

    {
      std::unique_lock<std::mutex> ul(thread_data->c_mutex_);
      thread_data->c_var_.wait(ul, [&](){
           return !thread_data->c_isWait_;
        });
      thread_data->c_isWait_ = true;
    }

    while (!thread_data->q_workfun_.empty()){
      std::unique_lock<std::mutex> ul(thread_data->q_workfun_mutex_);
      auto func = thread_data->q_workfun_.front();
      thread_data->q_workfun_.pop();
      ul.unlock();
      func();
    }

    if (!thread_data->isContine_.load()) {
      break;
    }
    
    {
      std::unique_lock<std::mutex> ul(c_mutex_);
      c_isWait_ = false;
      c_var_.notify_one();
    }

  }

  YILOG_TRACE("func: {}", __func__);

}


}
