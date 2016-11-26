#ifndef MONGO_H_
#define MONGO_H_

#include "macro.h"
#include <iostream>
#include <cstdint>
#include <vector>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include "protofiles/chat_message.pb.h"


class mongo_client {
public:
  mongo_client();
  ~mongo_client();

  // user
  void insertUser(const chat::Register & enroll);

  // mongo pull device push device
//  void upsertDevice(const std::string & userID, const chat::Device & device);
  void updateDevice(const std::string & userID, const chat::Device & device);
  void insertDevice(const std::string & userID, const chat::Device & device);

  std::shared_ptr<chat::User> 
  queryUser(const std::string & phoneNo, const std::string & countryCode);
  
  
  std::shared_ptr<chat::User> 
  queryUser(const std::string & userID);

  std::shared_ptr<chat::AddFriendAuthorizeInfo>
  addFriendAuthorize(const std::string & inviter, 
      const std::string & inviterNickname,
      const std::string & invitee,
      const std::string & inviteeNickname);

  // group  
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
  // in incomplete message, out complete message
  // add incrementID value
  std::shared_ptr<chat::NodeMessageRes> 
  insertMessage(chat::NodeMessage & message);

  std::shared_ptr<chat::NodeMessage>
  queryMessage(chat::NodeMessageRes & nodeRes);

  std::shared_ptr<chat::NodeMessage>
  queryMessage(chat::QueryOneMessage & query);

  std::shared_ptr<mongocxx::cursor>
  cursor(chat::QueryMessage & query);

  std::shared_ptr<chat::NodeMessage>
  queryMessage(std::shared_ptr<mongocxx::cursor> cursor_sp);

private:

  mongocxx::client client_;

};

class inmem_client {
public:
  inmem_client(std::string serverName);
  ~inmem_client();

  // query current server device connect info
  void devices(const chat::NodeSpecifiy& node_specifiy, 
      std::function<void(chat::ConnectInfoLittle&)> && func);
  void devices(const chat::NodeUser & node_user, 
      std::function<void(chat::ConnectInfoLittle&)> && func);
  
  void removeConnectInfo(const std::string & uuid);
  void insertConnectInfo(const chat::ConnectInfo & connectInfo);
  // spectify device add tonodeids
  void addTonodeidConnectInfo(const chat::ConnectInfo & connectInfo);
  // user's all device add tonodeid
  template <class Vec_like> void 
  addTonodeidConnectInfo(
      const Vec_like &  membersid, const std::string & tonodeid) {

    YILOG_TRACE ("func: {}. ", __func__);
    
    auto db = client_["chatdb"];
    auto connectinfo_col = db["connectInfo"];
    auto arraybuilder = bsoncxx::builder::stream::array{};
    for (auto & memberid: membersid) {
      arraybuilder << memberid;
    }
    auto member_array = arraybuilder 
        << bsoncxx::builder::stream::finalize;

    auto maybe_result = connectinfo_col.update_many(
        bsoncxx::builder::stream::document{} 
        << "userID" << bsoncxx::builder::stream::open_document
        << "$in" << member_array 
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize,
        bsoncxx::builder::stream::document{} 
        << "$addToSet" << bsoncxx::builder::stream::open_document
        << "toNodeIDs" << tonodeid
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize
        );
    if (unlikely(!maybe_result)) {
      for (auto & memberid: membersid) {
        YILOG_ERROR("user's devices add tonodeid error, userid :{} ,"
            " tonodeid :{}", memberid, tonodeid);
      }
    }
  }

  void updateConnectInfo(const chat::ConnectInfoLittle & infolittle);
  void disconnectInfo(const std::string & uuid);
  
private:

  std::string serverName_;
  mongocxx::client client_;

};

namespace yijian {
  namespace threadCurrent {
    mongo_client * mongoClient();
    inmem_client * inmemClient();
  }
}


#endif
