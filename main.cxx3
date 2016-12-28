#define CATCH_CONFIG_MAIN
#include "macro.h"
#include "catch.hpp"
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

#ifdef __cpluscplus
extern "C" {
#endif


  /*
TEST_CASE("buffer", "[buffer]") {
  initConsoleLog();
  SECTION("encoding | decoing var length") {
    for (unsigned int i = 0; i < 2097152; ++i) {
      uint32_t length = i;
      char data[1024];
      auto buffer_sp = new yijian::buffer();
      buffer_sp->encoding_var_length(data, i);
      auto pair = buffer_sp->decoding_var_length(data);
      REQUIRE( i == pair.first);
    }
  }
}
  */

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


TEST_CASE("IM business","[business]") {
  initConsoleLog();
  using yijian::Buffer_SP;

  auto thread_id = std::this_thread::get_id();
  std::cout << "main thread id " << thread_id << std::endl;

  typedef boost::coroutines2::coroutine<chat::NodeMessage> coro_t;
  coro_t::pull_type getOneMsg(
      [](coro_t::push_type & sink){
        auto msg = chat::NodeMessage();
        for (int i = 0; i < 1000; ++i) {
          
        }
      });

  create_client([](Buffer_SP buffer_sp) {
        // register login logout connect disconnect
        YILOG_TRACE("client callback regist");
        YILOG_TRACE("client callback duregist");
        YILOG_TRACE("client callback login");
        YILOG_TRACE("client callback logout");
        YILOG_TRACE("client callback login");
        YILOG_TRACE("client callback connect");
        YILOG_TRACE("client callback logout");
        YILOG_TRACE("client callback login");
        YILOG_TRACE("client callback connect");
        YILOG_TRACE("client callback disconnect");
        YILOG_TRACE("client callback logout");
        notimain();
      });

  SECTION("register") {
    auto regst = chat::Register();
    regst.set_phoneno("18514029918");
    regst.set_countrycode("86");
    regst.set_password("123456");
    regst.set_nickname("yijian");
    auto sp = yijian::buffer::Buffer(regst);
    sleep(1);
    client_send(sp, nullptr);
    mainwait();
    client_send(sp, nullptr);
    mainwait();
    auto msgTest = chat::NodeMessage();
    client_send(yijian::buffer::Buffer(msgTest), nullptr);
  }

}


#ifdef __cpluscplus
}
#endif

