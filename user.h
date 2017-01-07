#ifndef USER_H
#define USER_H
#include <google/protobuf/util/json_util.h>
#include "buffer.h"
#include "spdlog/spdlog.h"
#include "yijian/lib_client.h"
#include "yijian/mongo.h"
#include "yijian/protofiles/chat_message.pb.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <condition_variable>

using google::protobuf::util::MessageToJsonString;;
template <typename Proto>
std::string pro2string(Proto & any) {
  std::string value;
  google::protobuf::util::MessageToJsonString(any, &value);
  return value;
}


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
        REQUIRE(db->disconnectKey() == key);
        notimain();
      });
  mainwait();
};

auto dbHasUserblock = [](kvdb * db, std::string userid) -> bool {
  std::string user_data;
  return db->getUser(userid, user_data);
};

auto queryuserblock = [](kvdb * db, std::string userid){
  std::string user_data;
  db->queryuser(std::move(userid), 
      [&db, &user_data, &userid](const std::string & key){
        REQUIRE(key == db->userKey(userid));
        db->get(key, user_data);
        chat::User user;
        user.ParseFromString(user_data);
        YILOG_INFO ("key: {}, user : {}", key, pro2string(user));
        notimain();
      });
  mainwait();
};

auto queryuserWithphoneblock = [](kvdb * db, std::string phoneno){
  db->queryuser(std::move(phoneno), "86", 
      [&db](const std::string & key){
        std::string user_data;
        db->get(key, user_data);
        chat::User user;
        user.ParseFromString(user_data);
        YILOG_INFO ("key: {}, user : {}", key, pro2string(user));
        notimain();
      });
  mainwait();
};

auto queryuserversionblock = [](kvdb * db, std::string userid) {
  db->queryuserVersion(std::move(userid),
      [](const std::string & version){
        YILOG_INFO ("query user version key {}", version);
        notimain();
      });
  mainwait();
};

auto userinfonotiblock = [](kvdb * db, std::string & value) {
  db->userInfoNoti([&db, &value](const std::string & key) {
        YILOG_INFO("wait add friend noti key {}.", key);
        db->get(key, value);
        if (key == db->addFriendNotiKey()) {
          chat::AddFriendNoti noti;
          noti.ParseFromString(value);
          YILOG_INFO ("add friend {}", pro2string(noti));
        }else if(key == db->loginNotiKey()){
          chat::LoginNoti noti;
          noti.ParseFromString(value);
          YILOG_INFO ("login {}", pro2string(noti));
        }else if(key == db->addFriendAuthorizeNotiKey()){
          chat::AddFriendAuthorizeNoti noti;
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

auto addfriendblock = [](kvdb * db, std::string friend_id) {
  db->addfriend(friend_id, "ni hao", [&db](const std::string & key){
        std::string value;
        db->get(key, value);
        chat::AddFriendRes res;
        res.ParseFromString(value);
        YILOG_INFO ("addfriend key: {}, value: {}", 
            key, 
            pro2string(res));
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

auto messagereceiveblock = [](kvdb * db) {
  db->
};

#endif
