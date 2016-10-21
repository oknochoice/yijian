#include "chat_mongo.h"

mongo_client::mongo_client() {
  mongocxx::uri uri("mongodb://localhost:27017");
  client_ = mongocxx::client(uri);
}


mongo_client::~mongo_client () {
}
