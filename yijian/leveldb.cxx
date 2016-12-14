#include "leveldb.h"
#include <system_error>

leveldb::leveldb(std::string & path) {
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, path, &db_);
  if (!status.ok()) {
    throw std::system_error(std::error_code(50000, std::generic_category()),
       "open db failure");
  }
}

leveldb::~leveldb() {
  delete db;
}
