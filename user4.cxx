#define CATCH_CONFIG_RUNNER
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "yijian/lib_client.h"
#include "yijian/mongo.h"
#include "yijian/protofiles/chat_message.pb.h"
#include <google/protobuf/util/json_util.h>
#include "buffer_yi.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <condition_variable>
#include "kvdb.h"
#include <google/protobuf/util/json_util.h>

#include <boost/coroutine2/all.hpp>


std::string phoneno = "18514029911";
std::string countrycode = "86";
std::string password = "123456";
std::string verifycode = "654321";
std::string name = "user4_yijian";
int os = 0;
std::string osversion = "9.0.3";
std::string appversion = "1.0.0";
std::string devicemodel = "iphone 5";
std::string uuid = phoneno + "_uuid";

std::string addfriendphone = "18514029910";
kvdb * db = nullptr;

#include "user.h"

int main(int argc, char* const argv[]) {
  initConsoleLog();
  db = new kvdb(name);
  Catch::Session session;
  int reCode = session.applyCommandLine(argc, argv);
  if ( 0 != reCode) {
    return reCode; 
  }

  session.run();

  delete db;

  return 0;
}


TEST_CASE("register login connect disconnet", "[register]") {


  SECTION("register") {
    db->registUser(phoneno, countrycode, password,
        verifycode, name, [](const std::string & key){
          std::string value;
          db->get(key, value);
          chat::RegisterRes res;
          res.ParseFromString(value);
          YILOG_INFO("key:{}, value:{}.", key, pro2string(res));
          REQUIRE(res.issuccess() == true);
          notimain();
        });
    mainwait();
  }

  SECTION("re register") {
    db->registUser(phoneno, countrycode, password,
        verifycode, name, [](const std::string & key){
          std::string value;
          db->get(key, value);
          chat::RegisterRes res;
          res.ParseFromString(value);
          YILOG_INFO("key:{}, value:{}.", key, pro2string(res));
          REQUIRE(res.issuccess() == false);
          notimain();
        });
    mainwait();
  }

  SECTION("login failure") {
    db->login(phoneno, countrycode, "000000", os, devicemodel, uuid,
        [](const std::string & key){
          std::string value;
          db->get(key, value);
          chat::LoginRes res;
          res.ParseFromString(value);
          YILOG_INFO("key:{}, value:{}.", key, pro2string(res));
          REQUIRE(res.issuccess() == false);
          notimain();
        });
    mainwait();
  }

  SECTION("login success") {
    loginblock(db);
  }

  SECTION("connect") {
    connectblock(db);
  }

  SECTION("disconnet") {
    disconnectblock(db);
  }


}

TEST_CASE("normal disconnet", "[connect]") {
  SECTION("login") {
    loginblock(db);
  }
  SECTION("connect") {
    connectblock(db);
  }
  SECTION("disconnet") {
    disconnectblock(db);
  }
}

TEST_CASE("abnormal disconnet", "[connect]") {
  SECTION("login") {
    loginblock(db);
  }
  SECTION("connect") {
    connectblock(db);
  }
}

TEST_CASE("wait for add friends & authorize", "[waitaddfriend]") {
  SECTION("login") {
    loginblock(db);
  }
  SECTION("connect") {
    connectblock(db);
  }
  SECTION("han user") {
    dbHasUserblock(db, db->get_current_userid());
  }
  SECTION("query user") {
    queryuserblock(db, db->get_current_userid());
  }
  SECTION("han user") {
    dbHasUserblock(db, db->get_current_userid());
  }
  SECTION("queryuser version") {
    queryuserversionblock(db, db->get_current_userid());
  }
  SECTION("wait user add noti") {
    std::string value;
    userinfonotiblock(db, value);
    chat::AddFriendNoti noti;
    noti.ParseFromString(value);
    REQUIRE(noti.response().inviteeid() == db->get_current_userid());
    SECTION("query AddFriendInfo") {
      qaddfrdinfo(db, noti.response().inviterid());
      SECTION("authorize addfriend") {
        addfriendauthorizeblock(db, noti.response().inviterid());
      }
    }
  }
  SECTION("disconnet") {
    disconnectblock(db);
  }
}

TEST_CASE("add friend & wait for authorize", "[addfriend]") {
  SECTION("login") {
    loginblock(db);
  }
  SECTION("connect") {
    connectblock(db);
  }
  SECTION("han user") {
    dbHasUserblock(db, db->get_current_userid());
  }
  SECTION("query user") {
    queryuserblock(db, db->get_current_userid());
  }
  SECTION("han user") {
    dbHasUserblock(db, db->get_current_userid());
  }
  SECTION("queryuser version") {
    queryuserversionblock(db, db->get_current_userid());
  }
  SECTION("query add friend info") {
    queryuserWithphoneblock(db, addfriendphone);
  }
  SECTION("add friend") {
    std::string user_data;
    db->getUser("86", addfriendphone, user_data);
    chat::User user;
    user.ParseFromString(user_data);
    addfriendblock(db, user.id());
  }
  SECTION("disconnet") {
    disconnectblock(db);
  }
}

/*
void test() {
  typedef std::shared_ptr<yijian::buffer> Buffer_SP;

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


}
*/

