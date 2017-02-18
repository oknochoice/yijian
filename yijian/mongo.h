#ifndef MONGO_H_
#define MONGO_H_

#include "macro.h"
#include <iostream>
#include <cstdint>
#include <vector>
#include <openssl/sha.h>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/bulk_write.hpp>
#include <mongocxx/write_concern.hpp>

#include "protofiles/chat_message.pb.h"


class mongo_client {
public:
  mongo_client(std::string serverName);
  ~mongo_client();

  // user
  // 40000 40010
  std::string& insertUser(const chat::Register & enroll);

  // mongo pull device push device
//  void upsertDevice(const std::string & userID, const chat::Device & device);
  void updateDevice(const std::string & userID, const chat::Device & device);
  void insertDevice(const std::string & userID, const chat::Device & device);

  std::shared_ptr<chat::User> 
    queryUser(const std::string & phoneNo, const std::string & countryCode);
  
  std::shared_ptr<chat::User> 
    queryUser(const std::string & userID);

  void setUserProperty(const std::string & userID, const chat::SetUserProperty & property);

  /*
  template <class Vec_like> 
  void loginRecord(const std::string & countryCode, 
                   const std::string & phoneNo,
                   const Vec_like & ips,
                   const bool isPass) {
    YILOG_TRACE ("func: {}. ", __func__);

    auto db = client_["chatdb"];
    auto login_col = db["userLogin"];
    auto arraybuilder = bsoncxx::builder::stream::array{};
    for (auto & ip: ips) {
      arraybuilder << ip;
    }
    auto ip_array = arraybuilder
        << bsoncxx::builder::stream::finalize;
    auto code_phone = countryCode + "_" + phoneNo;
    auto maybe_result = login_col.insert_one(
        bsoncxx::builder::stream::document{} 
        << "code_phone" << code_phone
        << "ip" << ip_array
        << "isPass" << isPass
        << bsoncxx::builder::stream::finalize);
    if (unlikely(!maybe_result)) {
      YILOG_ERROR ("login record failure user code phone: {}, isPass: {}",
          code_phone, isPass);
    }
  }

  template <class Vec_like> 
  void connectRecord(const std::string & userID, 
                     const std::string & uuid, 
                     const Vec_like & ips) {
    YILOG_TRACE ("func: {}. ", __func__);
    auto db = client_["chatdb"];
    auto connect_col = db["userConnect"];
    auto arraybuilder = bsoncxx::builder::stream::array{};
    for (auto & ip: ips) {
      arraybuilder << ip;
    }
    auto ip_array = arraybuilder
        << bsoncxx::builder::stream::finalize;
    auto maybe_result = connect_col.insert_one(
        bsoncxx::builder::stream::document{} 
        << "userID" << userID
        << "UUID" << uuid
        << "ip" << ip_array
        << bsoncxx::builder::stream::finalize);
    if (unlikely(!maybe_result)) {
      YILOG_ERROR ("login record failure userid: {}, uuid: {}",
          userID, uuid);
    }
  }
  */

  std::shared_ptr<chat::AddFriendRes>
    addFriend(const chat::AddFriend & addfrd);

  void addFriendAuthorize(const std::string & inviter, 
      const std::string & invitee);

  void queryAddfriendInfo(
      std::function<void(chat::AddFriendInfo &)> && func,
      const std::string & userid,
      const int limit = 10);

  // group  
  // 40020
  // insert all memeber group id
  std::shared_ptr<chat::CreateGroupRes>
    createGroup(const chat::CreateGroup & cGroup);

  // insert all memeber group info
  std::shared_ptr<chat::GroupAddMemberRes>
    addMembers2Group(chat::GroupAddMember & groupMember);

  // node
  std::shared_ptr<chat::MessageNode>
    queryNode(const std::string & nodeID);

  // message
  // 40050
  // in incomplete message, out complete message
  // add incrementID value
  std::shared_ptr<chat::NodeMessageRes> 
    insertMessage(chat::NodeMessage & message);

  std::shared_ptr<chat::NodeMessage>
    queryMessage(const std::string & tonodeid, const int32_t incrementid);

  void queryMessage(chat::QueryMessage & query, 
      std::function<void(chat::NodeMessage&)> && func);


  // media
  // 40060
  void insertMedia(const chat::Media & media);
  void queryMedia(const std::string & sha1, 
      chat::Media & media);

  // query current server device connect info
//  void devices(const chat::NodeSpecifiy& node_specifiy, 
//      std::function<void(chat::ConnectInfoLittle&)> && func);
  void devices(const chat::NodeUser & node_user, 
      std::function<void(chat::ConnectInfoLittle&)> && func);

  template <class Vec_like_String> 
  void devices(const Vec_like_String & membersid, 
      const std::string & currentUserid,
      std::function<void(chat::ConnectInfoLittle&)> && func) {
    auto db = client_["chatdb"];
    auto connectinfo_col = db["connectInfo"];
    auto arraybuilder = bsoncxx::builder::stream::array{};
    for (auto & memberid: membersid) {
      if (likely(memberid != currentUserid)) {
        arraybuilder << memberid;
      }
    }
    auto member_array = arraybuilder 
        << bsoncxx::builder::stream::finalize;

    auto cursor = connectinfo_col.find(
        bsoncxx::builder::stream::document{} 
        << "userID" << 
        bsoncxx::builder::stream::open_document
        << "$in" << member_array << 
        bsoncxx::builder::stream::close_document 
        << "serverName" << serverName_
        << "isLogin" << true
        << bsoncxx::builder::stream::finalize);
    auto infolittle = chat::ConnectInfoLittle();
    for (auto doc: cursor) {
      infolittle.set_uuid(doc["UUID"].get_utf8().value.to_string());
      infolittle.set_isconnected(doc["isConnected"].get_bool().value);
      infolittle.set_isrecivenoti(doc["isReciveNoti"].get_bool().value);
      func(infolittle);
    }
  }
  
  // 40030
  void insertUUID(const chat::ConnectInfo & connectInfo);
  void updateUUID(const chat::ConnectInfo & connectInfo);
  bool findUUID(const std::string & uuid, 
      chat::ConnectInfo & connectInfo);
  void updateSessionID(const std::string & uuid,
      const std::string & userid,
      const uint16_t sessionid);

  //  user unread node
  void insertUnreadNode(const std::string & userid,
                  const std::string & tonodeid);
  void updateReadedIncrement(const std::string & userid,
                             const std::string & tonodeid,
                             const int32_t readedIncrement);
  template <class Vec_like_String> 
  void updateUnreadIncrement(const Vec_like_String & userids,
                             const std::string & currentUserid,
                             const std::string & tonodeid,
                             const int32_t unreadIncrement) {
    YILOG_TRACE ("func: {}. ", __func__);
    auto db = client_["chatdb"];
    auto userUnread = db["userUnread"];
    auto arraybuilder = bsoncxx::builder::stream::array{};
    for (auto & memberid: userids) {
      if (likely(memberid != currentUserid)) {
        arraybuilder << memberid;
      }
    }
    auto member_array = arraybuilder 
        << bsoncxx::builder::stream::finalize;
    try {
    userUnread.update_many(
        bsoncxx::builder::stream::document{} 
        << "userID" 
        << bsoncxx::builder::stream::open_document
        << "$in" << member_array 
        << bsoncxx::builder::stream::close_document 
        << bsoncxx::builder::stream::finalize,
        bsoncxx::builder::stream::document{} 
        << "$set" 
        << bsoncxx::builder::stream::open_document
        << "unreadIncrement" << unreadIncrement
        << bsoncxx::builder::stream::close_document 
        << bsoncxx::builder::stream::finalize);
    }catch(std::system_error & e) {
      YILOG_ERROR ("update unread node failure.\n"
          "system_error code:{}, what{}.",
          e.code().value(), e.what());
    }
  }
  void updateUnreadIncrement(const std::string & userid,
                             const std::string & tonodeid,
                             const int32_t unreadIncrement);
  void unreadNodes(const std::string & userid,
     std::function<void(const std::string & tonodeid,
                        const int32_t unreadIncrement,
                        const int32_t readedIncrement)> && func);

private:
  std::string serverName_;
private:

  mongocxx::client client_;
  mongocxx::write_concern journal_concern_;
  mongocxx::options::insert journal_insert_;
  mongocxx::options::update journal_update_;

};

namespace yijian {
  namespace threadCurrent {
    std::shared_ptr<mongo_client> mongoClient();
  }
}


#endif
