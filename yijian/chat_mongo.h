#ifndef CHAT_MONGO_H_
#define CHAT_MONGO_H_

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
  void enrollUser(chat::Register && user);
  chat::User && login(chat::Login && login);
  void logout(chat::Logout && logout);
  chat::User && queryUser(std::string && userID);
  void incrementUserUnread(std::string && userID, std::string && nodeID);
  void cleanUserUnread(std::string && userID, std::string && nodeID);

  // message
  std::string && insertOne(chat::ChatMessage && message);

  chat::ChatMessage && queryMessageOne(std::string && messageID);

  mongocxx::cursor && queryNodeOne(uint_fast32_t incrementID);
  mongocxx::cursor && queryNode(uint_fast32_t fromIncrementID, 
      uint_fast32_t toIncrementID);
  mongocxx::cursor && queryNode(uint_fast32_t toIncrementID);

private:

  mongocxx::client client_;

}



#endif
