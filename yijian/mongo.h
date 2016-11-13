#ifndef MONGO_H_
#define MONGO_H_

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

  std::shared_ptr<chat::QueryUserRes>
  queryUser(const chat::QueryUser & queryUser);

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

  std::shared_ptr<chat::QueryNodeRes>
  queryNode(const std::string & nodeID);

  // message
  // in incomplete message, out complete message
  // add incrementID value
  std::shared_ptr<chat::MessageUserRes> 
  insertMessage(chat::UserMessage & message);
  
  std::shared_ptr<chat::MessageNodeRes> 
  insertMessage(chat::NodeMessage & message);

  std::shared_ptr<chat::UserMessage>
  queryMessage(chat::MessageUserRes & userRes);

  std::shared_ptr<chat::NodeMessage>
  queryMessage(chat::MessageNodeRes & nodeRes);

  // message node 
  std::shared_ptr<chat::MessageNode>
  insertToNode(const std::string & userID, const std::string & friendID);

private:

  mongocxx::client client_;

}

class inmem_client {
public:
  inmem_client(std::string serverName);
  ~inmem_client();

  // query current server device connect info
  mongocxx::cursor devices(const chat::NodeSpecifiy& node_specifiy,
      const std::string && serverName);

  mongocxx::cursor devices(const chat::NodeUser & node_user,
      const std::string && serverName);
  
  void removeConnectInfos(const std::string & UUID);

  void insertConnectInfo(const chat::ConnectInfo & connectInfo);
  
private:

  mongocxx::client client_;

}

namespace yijian {
  namespace threadCurrent {
    mongo_client * mongoClient();
    inmem_client * inmemClient();
  }
}


#endif
