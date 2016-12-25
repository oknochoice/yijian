#include "macro.h"
#include "spdlog/spdlog.h"
#include "yijian/lib_client.h"
#include "yijian/mongo.h"
#include "yijian/protofiles/chat_message.pb.h"
#include <google/protobuf/util/json_util.h>
#include "buffer.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <condition_variable>
#include "kvdb.h"

#include <boost/coroutine2/all.hpp>
std::mutex mutex_;
std::condition_variable cvar_;
bool iswait_ = true;

void mainwait() {
  YILOG_TRACE("func: {}, iswait_: {}", __func__, iswait_);
  std::unique_lock<std::mutex> ul(mutex_);
  cvar_.wait(ul, [](){
        YILOG_TRACE("iswait_: {}", iswait_);
        return !iswait_;
      });
  iswait_ = true;
}

void notimain() {
  YILOG_TRACE("func: {}, iswait_: {}", __func__, iswait_);
  std::unique_lock<std::mutex> ul(mutex_);
  iswait_ = false;
  cvar_.notify_one();
}

int main() {
  initConsoleLog();
  using yijian::Buffer_SP;

  auto thread_id = std::this_thread::get_id();
  std::cout << "main thread id " << thread_id << std::endl;

  typedef boost::coroutines2::coroutine<chat::NodeMessage> coro_t;
  coro_t::pull_type getOneMsg(
      [](coro_t::push_type & sink){
        auto msg = chat::NodeMessage();
        for (int i = 0; i < 1000; ++i) {
          msg.set_type(chat::MediaType::TEXT);
          msg.set_content(std::to_string(i));
          sink(msg);
        }
      });
  for (int i = 0; i < 10; ++i) {
    auto item = getOneMsg.get();
    getOneMsg();
    YILOG_TRACE("content:{}.", item.content());
  }

  std::string name = "user1_yijian";
  auto db = new kvdb(name);

  db->registUser("18514029918", "86", "123456",
      "213465", name, [&db](const std::string & key){
        std::string value;
        db->get(key, value);
        YILOG_INFO("key:{}, value:{}.", key, value);
        chat::RegisterRes res;
        res.ParseFromString(value);
        assert(res.issuccess() == true);
        notimain();
      });
  mainwait();

  db->registUser("18514029918", "86", "123456",
      "213465", "yijian", [&db](const std::string & key){
        YILOG_INFO("key:{}.", key);
        std::string value;
        db->get(key, value);
        chat::RegisterRes res;
        res.ParseFromString(value);
        assert(res.issuccess() == false);
        YILOG_INFO("errno:{}, msg:{}.", 
            res.e_no(), res.e_msg());
        notimain();
      });
  mainwait();

  return 0;
}

