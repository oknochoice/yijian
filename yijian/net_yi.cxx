#include "net_yi.h"

enum Session_ID : int32_t {
  regist_login_connect = -1,
  logout_disconnect = -2,
  accept_unread_msg = -3,
  user_noti = -4
};

netyi::netyi(std::string & path) {
  YILOG_TRACE ("func: {}", __func__);
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, path, &db_);

  if (!status.ok()) {
    throw std::system_error(std::error_code(55000, 
          std::generic_category()),
       "open db failure");
  }

  create_client([&](Buffer_SP sp){
    YILOG_TRACE ("net callback");
    dispatch(sp->datatype(), sp);
  });
}

