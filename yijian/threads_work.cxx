#include "threads_work.h"
#include "stdlib.h"

namespace yijian {

// noti threads
noti_threads::noti_threads(uint_fast16_t thread_num)
  : thread_count_(thread_num){
  YILOG_TRACE("func: {}. thread num {}. ", __func__, thread_num);
    for (uint_fast16_t i = 0; i < thread_num; ++i) {
      auto thread_data = std::make_shared<Thread_Data>();
      thread_data->isContine_ = true;
      thread_data->c_isWait_ = true;
      thread_data->v_pingnode_sp_.reset(new Thread_Data::Vector_Node);
      thread_data->thread_ = 
//        std::thread(&noti_threads::thread_func, thread_data);
          std::thread([this, thread_data]() {
                YILOG_TRACE("start thread");
                this->thread_func(thread_data);
              });
      vec_threads_.push_back(thread_data);
    }
};

noti_threads::~noti_threads() {
  YILOG_TRACE("func: {}", __func__);
  for(auto thread_data: vec_threads_) {
    thread_data->thread_.join();
  }
}

void noti_threads::sentWork(Thread_Data::Thread_Function && func) {

  YILOG_TRACE("func: {}", __func__);

  bool isContine = true;

  while(isContine) {

    // select min work size
    std::shared_ptr<Thread_Data> thread_local_data = nullptr;
    for (auto & thread_data: vec_threads_) {
      std::unique_lock<std::mutex> ul(thread_data->q_workfun_mutex_);
      if (thread_local_data == nullptr) {
        std::atomic_store(&thread_local_data, thread_data);
        continue;
      }
      if (thread_local_data->q_workfun_.size() > 
          thread_data->q_workfun_.size()) {
        std::atomic_store(&thread_local_data, thread_data);
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
      c_var_.wait(ul, [this](){
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
    std::function<void(std::shared_ptr<Read_IO>)> && func) {

  YILOG_TRACE("func: {}", __func__);
  for (auto & thread_data: vec_threads_) {
    Thread_Data::Vector_Node_SP data_sp;
    {
      std::unique_lock<std::mutex> ul(thread_data->v_pingnode_mutex_);
      data_sp = thread_data->v_pingnode_sp_;
      thread_data->v_pingnode_sp_.reset(new Thread_Data::Vector_Node);
    }
    YILOG_TRACE("func: {}, node count {}", __func__, data_sp->size());
    for (auto node: *data_sp) {
      func(node);
    }
  }

}

int noti_threads::taskCount() {
  YILOG_TRACE("func: {}", __func__);
  int count = 0;
  for (auto & thread_data: vec_threads_) {
    Thread_Data::Vector_Node_SP data_sp;
    {
      std::unique_lock<std::mutex> ul(thread_data->v_pingnode_mutex_);
      data_sp = thread_data->v_pingnode_sp_;
    }
    YILOG_TRACE("func: {}, node count {}", __func__, data_sp->size());
    for (auto node: *data_sp) {
      ++count;
    }
  }
  return count;
}

namespace threadCurrent {

std::shared_ptr<Thread_Data> 
threadData(std::shared_ptr<Thread_Data> currentData) {
  YILOG_TRACE("func: {}", __func__);
  static thread_local auto threaddata = currentData;
  return threaddata;
}

std::shared_ptr<Thread_Data> threadData() {
  YILOG_TRACE("func: {}", __func__);
  return threadData(nullptr);
}

void pushPingnode(std::shared_ptr<Read_IO> node) {
  YILOG_TRACE("func: {}", __func__);

  auto thread_data = threadData();
  std::unique_lock<std::mutex> ul(thread_data->v_pingnode_mutex_);
  thread_data->v_pingnode_sp_->push_back(node);

}


}

void noti_threads::thread_func(std::shared_ptr<Thread_Data> thread_data) {

  YILOG_TRACE("func: {}", __func__);
  threadCurrent::threadData(thread_data);
  
  while (true) {

    {
      YILOG_TRACE("func: {}, wait...", __func__);
      std::unique_lock<std::mutex> ul(thread_data->c_mutex_);
      thread_data->c_var_.wait(ul, [thread_data](){
           return !thread_data->c_isWait_;
        });
      thread_data->c_isWait_ = true;
    }

    while (!thread_data->q_workfun_.empty()){
      YILOG_TRACE("func: {}, do work", __func__);
      std::unique_lock<std::mutex> ul(thread_data->q_workfun_mutex_);
      auto func = std::forward<Thread_Data::Thread_Function>(
          thread_data->q_workfun_.front());
      thread_data->q_workfun_.pop();
      ul.unlock();
      func();
    }

    if (!thread_data->isContine_.load()) {
      YILOG_TRACE("func: {}, stop thread", __func__);
      break;
    }
    
    {
      YILOG_TRACE("func: {}, notify_one", __func__);
      std::unique_lock<std::mutex> ul(c_mutex_);
      c_isWait_ = false;
      c_var_.notify_one();
    }

  }

  YILOG_TRACE("func: {}, outer loop", __func__);

}


}
