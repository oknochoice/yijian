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

#ifdef __cpluscplus
extern "C" {
#endif

TEST_CASE("buffer", "[buffer]") {
  initConsoleLog();
  SECTION("encoding | decoing var length") {
    uint32_t length = 33;
    char data[1024];
    auto buffer_sp = new yijian::buffer();
    buffer_sp->encoding_var_length(data, length);
    auto pair = buffer_sp->decoding_var_length(data);
    REQUIRE( length == pair.first);
  }
}


TEST_CASE("IM business","[business]") {

  create_client([](Buffer_SP buffer_sp) {
      YILOG_TRACE ("client read callback");
      auto datatype = buffer_sp->datatype();
      auto datasize = buffer_sp->data_size();
      auto size = buffer_sp->size();
      YILOG_TRACE ("buffer datatype {}, datasize {}, size {}", 
          datatype, datasize, size);
      auto error = chat::Error();
      error.ParseFromArray(buffer_sp->data(), buffer_sp->data_size());
      std::string error_string;
      google::protobuf::util::MessageToJsonString(error, &error_string);
      YILOG_TRACE("error: {}", error_string);
      YILOG_TRACE("error num: {}, msg: {}", error.errnum(), error.errmsg());
      });



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

