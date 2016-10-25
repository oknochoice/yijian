#include "threads_work.h"
#include "stdlib.h"

namespace yijian {

// noti threads
noti_threads::noti_threads(uint_fast16_t thread_num)
  : thread_count_(thread_num){
    for (uint_fast16_t i = 0; i < thread_num; i++) {
      auto thread_data = (Thread_Data*)malloc(sizeof(Thread_Data));
      thread_data->thread_ = std::thread(&noti_threads::thread_func, thread_data);
      thread_data->isContine_ = true;
      thread_data->c_isWait_ = true;
      thread_data->v_pingnode_sp_.reset(new Thread_Data::Vector_Node);
      vec_threads_.push_back(thread_data);
    }
};

noti_threads::~noti_threads() {
  for(auto thread_data: vec_threads_) {
    thread_data->thread_.join();
    free(thread_data);
  }
}

void noti_threads::sentWork(std::function<void(void)> && func,
    PingNode * node, 
    std::function<void(void)> && finish) {
  bool isContine = true;
  while(isContine) {

    for (auto thread_data: vec_threads_) {
      if (nullptr == thread_data->current_node_) {
        thread_data->work_func_ = func;
        thread_data->workFinish_func_ = finish;
        thread_data->current_node_ = node;
        isContine = false;
        std::unique_lock<std::mutex> ul(thread_data->c_mutex_);
        thread_data->c_isWait_ = false;
        thread_data->c_var_.notify_one();
        break;
      }
    }

    if (isContine) {
      std::unique_lock<std::mutex> ul(c_mutex_);
      c_var_.wait(ul, [&](){
          return !c_isWait_;
          });
      c_isWait_ = true;
    }

  }
}

void noti_threads::foreachio(
    std::function<void(struct PingNode *)> && func) {

  for (auto thread_data: vec_threads_) {
    Thread_Data::Vector_Node_SP data_sp;
    {
      std::unique_lock<std::mutex> ul(thread_data->v_mutex_);
      data_sp = thread_data->v_pingnode_sp_;
    }
    for (auto node: *data_sp) {
      func(node);
    }
  }

}

namespace threadCurrent {
Thread_Data * threadData(Thread_Data* currentData) {
  static thread_local auto threaddata = currentData;
  return threaddata;
}
Thread_Data * threadData() {
  return threadData(nullptr);
}
}

void noti_threads::thread_func(Thread_Data* thread_data) {
  bool isContine = true;
  threadCurrent::threadData(thread_data);
  while (isContine) {
    
    if (thread_data->isContine_.load()){
      {
        std::unique_lock<std::mutex> ul(thread_data->c_mutex_);
        thread_data->c_var_.wait(ul, [&](){
             return !thread_data->c_isWait_;
          });
        thread_data->c_isWait_ = true;
      }

      thread_data->work_func_();

      {
        std::unique_lock<std::mutex> ul(thread_data->v_mutex_);
        auto v_pingnode = thread_data->v_pingnode_sp_;
        if (!v_pingnode.unique()) {
          v_pingnode.reset(new Thread_Data::Vector_Node);
        }
        v_pingnode->push_back(thread_data->current_node_);
        thread_data->current_node_ = nullptr;
      }

      {
        std::unique_lock<std::mutex> ul(c_mutex_);
        c_isWait_ = false;
        c_var_.notify_one();
      }

      thread_data->workFinish_func_();

    }else {
      if (thread_data->v_pingnode_sp_->empty()) {
        isContine = false;
      }else {
        std::this_thread::yield();
      }
    }
  }
}


}
