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

#ifdef __cpluscplus
extern "C" {
#endif


TEST_CASE("buffer", "[buffer]") {
  initConsoleLog();
  /*
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
  */
}

void im_cb(const char * header, uint32_t length, 
    uint16_t session, int type) {
  std::cout << "length " << length
    << ", session " << session
    << ", type " << type << std::endl;
  auto thread_id = std::this_thread::get_id();
  std::cout << "sub thread id " << thread_id << std::endl;
  std::cout << "content: " << std::string(header, length) << std::endl;
}


TEST_CASE("IM business","[business]") {

  auto thread_id = std::this_thread::get_id();
  std::cout << "main thread id " << thread_id << std::endl;
  create_client(im_cb);

  sleep(1);
  SECTION("register") {
    auto regst = chat::Register();
    regst.set_phoneno("18514029919");
    regst.set_countrycode("86");
    regst.set_password("123456");
    regst.set_nickname("yijian");
    auto sp = yijian::buffer::Buffer(regst);
    client_send(sp);
    YILOG_DEBUG ("size: {}", sp->size());
    sleep(10);
  }

}


#ifdef __cpluscplus
}
#endif

