#include "mongo.h"

mongo_client::mongo_client() {
  mongocxx::uri uri("mongodb://localhost:27017");
  client_ = mongocxx::client(uri);
}


mongo_client::~mongo_client () {
}


namespace yijian {
  namespace threadCurrent {
    mongo_client * mongoClient() {
      static thread_local auto client = new mongo_client();
      return client;
    }
    inmem_client * inmemClient() {
      static thread_local auto client = new inmem_client("1");
      return client;
    }
  }
}

