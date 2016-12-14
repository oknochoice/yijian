#ifndef LEVEL_DB_CLIENT_H
#define LEVEL_DB_CLIENT_H

#include <leveldb/db.h>

class leveldb {
public:
  leveldb(std::string & path);
  ~leveldb();
private:
  leveldb::DB * db_;
};


#endif
