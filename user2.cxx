#define CATCH_CONFIG_RUNNER
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
#include <google/protobuf/util/json_util.h>

#include <boost/coroutine2/all.hpp>

using google::protobuf::util::MessageToJsonString;;
template <typename Proto>
std::string pro2string(Proto & any) {
  std::string value;
  google::protobuf::util::MessageToJsonString(any, &value);
  return value;
}

std::string phoneno = "18514029919";
std::string countrycode = "86";
std::string password = "123456";
std::string verifycode = "654321";
std::string name = "user2_yijian";
int os = 0;
std::string osversion = "10.0.2";
std::string appversion = "1.0.0";
std::string devicemodel = "iphone 8";
std::string uuid = phoneno + "_uuid";

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
auto loginblock = [](kvdb * db) {
  db->login(phoneno, countrycode, password, os, devicemodel, uuid,
      [&db](const std::string & key){
        std::string value;
        db->get(key, value);
        chat::LoginRes res;
        res.ParseFromString(value);
        YILOG_INFO("key:{}, value:{}.", key, pro2string(res));
        REQUIRE(res.issuccess() == true);
        notimain();
      });
  mainwait();
};

auto connectblock = [](kvdb * db) {
  std::string userid = db->get_current_userid();
  db->connect(userid, uuid, true, osversion, appversion,
      [&db](const std::string & key){
        YILOG_INFO("key:{}.", key);
        std::string value;
        db->get(key, value);
        chat::ClientConnectRes res;
        res.ParseFromString(value);
        YILOG_INFO("key:{}, value:{}.", key, pro2string(res));
        REQUIRE(res.issuccess() == true);
        notimain();
      });
  mainwait();
};

auto disconnectblock = [](kvdb * db) {
  std::string userid = db->get_current_userid();
  db->disconnect(userid, uuid, 
      [&db](const std::string & key){
        std::string value;
        db->get(key, value);
        chat::ClientConnectRes res;
        res.ParseFromString(value);
        YILOG_INFO("key:{}, value:{}.", key, pro2string(res));
        REQUIRE(res.issuccess() == true);
        notimain();
      });
  mainwait();
};

auto queryuserblock = [](kvdb * db, std::string userid){
  std::string user_data;
  db->queryuser(std::move(userid), 
      [&db, &user_data, &userid](const std::string & key){
        assert(key == db->userKey(userid));
        db->get(key, user_data);
        chat::User user;
        user.ParseFromString(user_data);
        YILOG_INFO ("key: {}, user : {}", key, pro2string(user));
        notimain();
      });
  mainwait();
};

auto dbHasUserblock = [](kvdb * db, std::string userid) -> bool {
  std::string user_data;
  return db->getUser(userid, user_data);
};

auto queryuserversionblock = [](kvdb * db, std::string userid) {
  db->queryuserVersion(std::move(userid),
      [](const std::string & version){
        YILOG_INFO ("query user version key {}", version);
        notimain();
      });
  mainwait();
};

auto userInfoNotiblock = [](kvdb * db, std::string & value) {
  db->userInfoNoti([&db, &value](const std::string & key) {
        YILOG_INFO("wait add friend noti key {}.", key);
        db->get(key, value);
        if (key == db->addFriendNotiKey()) {
          chat::AddFriendNoti noti;
          noti.ParseFromString(value);
          YILOG_INFO ("add friend authorize {}", pro2string(noti));
        }else {
          YILOG_ERROR ("noti type error");
        }
        notimain();
      });
  mainwait();
};

auto qaddfrdinfo = [](kvdb * db, std::string friend_id){
  db->queryaddfriendinfo([&db, &friend_id](const std::string & key){
      YILOG_INFO ("query addfriend info key {}", key);
      std::string value;
      db->get(key, value);
      chat::AddFriendInfo info;
      info.ParseFromString(value);
      if (info.info().size() > 0)
        friend_id = info.info(0).inviter();
      YILOG_INFO ("add friend info {}", pro2string(info));
      notimain();
    });
  mainwait();
};

auto addfriendauthorizeblock = [](kvdb * db, std::string friend_id){
  db->addfriendAuthorize(friend_id, chat::IsAgree::agree, 
      [&db, &friend_id](const std::string & key){
        YILOG_INFO ("addfriendauthorize key {}", key);
        std::string value;
        db->get(key, value);
        chat::AddFriendAuthorizeRes res;
        res.ParseFromString(value);
        YILOG_INFO ("add friend authorize info {}", pro2string(res));
        notimain();
      });
  mainwait();
};

int main(int argc, char* const argv[]) {
  initConsoleLog();
  Catch::Session session;
  int reCode = session.applyCommandLine(argc, argv);
  if ( 0 != reCode) {
    return reCode; 
  }

  return session.run();
}


;

TEST_CASE("register login connect disconnet", "[register]") {

  auto db = new kvdb(name);

  SECTION("register") {
    db->registUser(phoneno, countrycode, password,
        verifycode, name, [&db](const std::string & key){
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
        verifycode, name, [&db](const std::string & key){
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
        [&db](const std::string & key){
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

  delete db;

}

TEST_CASE("normal disconnet", "[connect]") {
  auto db = new kvdb(name);
  SECTION("login") {
    loginblock(db);
  }
  SECTION("connect") {
    connectblock(db);
  }
  SECTION("disconnet") {
    disconnectblock(db);
  }
  delete db;
}

TEST_CASE("abnormal disconnet", "[connect]") {
  auto db = new kvdb(name);
  SECTION("login") {
    loginblock(db);
  }
  SECTION("connect") {
    connectblock(db);
  }
  delete db;
}

TEST_CASE("friends add & authorize", "[friends]") {
  auto db = new kvdb(name);
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
    userInfoNotiblock(db, value);
    chat::AddFriendNoti noti;
    noti.ParseFromString(value);
    REQUIRE(noti.response().inviteeid() == db->get_current_userid());
    SECTION("query AddFriendInfo") {
      qaddfrdinfo(db, noti.response().inviteeid());
    }
  }
  SECTION("disconnet") {
    disconnectblock(db);
  }
  delete db;
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

