#ifndef MONGO_H_
#define MONGO_H_

#include <iostream>
#include <cstdint>
#include <vector>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include "protofiles/chat_message.pb.h"


class mongo_client {
public:
  mongo_client();
  ~mongo_client();

  // user
  void enrollUser(const chat::User & user);
  chat::User && login(const chat::Login & login);
  void logout(const chat::Logout & logout);
  chat::User && queryUserID(const std::string & userID);
  bool isUserRollined(const std::string & phone, const std::string & countryCode);
  void incrementUserUnread(const std::string & userID, const std::string & nodeID);
  void cleanUserUnread(const std::string & userID, const std::string & nodeID);

  // message
  std::string && insertOne(const chat::ChatMessage & message);

  chat::ChatMessage && queryMessageOne(const std::string & messageID);

private:

  mongocxx::client client_;

}

class inmem_client {
public:
  inmem_client(std::string serverName);
  ~inmem_client();

  void insertOrUpdateDeivce(const chat::ConnectInfo & connectInfo);
  mongocxx::cursor devices(const std::string & chatMessageNode,
      const std::string && serverName);
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
