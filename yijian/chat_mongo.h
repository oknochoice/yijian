#ifndef CHAT_MONGO_H_
#define CHAT_MONGO_H_

#include <iostream>
#include <cstdint>
#include <vector>

#include <bsoncxx/v_noabi/bsoncxx/json.hpp>
#include <mongocxx/v_noabi/mongocxx/client.hpp>

class chat_mongo {
public:
  using bsoncxx::builder::stream::close_array;
  using bsoncxx::builder::stream::close_document;
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;
  using bsoncxx::builder::stream::open_array;
  using bsoncxx::builder::stream::open_document;
  chat_mongo();
  ~chat_mongo();
}


#endif
