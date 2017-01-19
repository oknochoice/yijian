#include "net_yi.h"
#include "macro.h"
#include "typemap.h"
#include "buffer_yi.h"
#include <leveldb/write_batch.h>

using yijian::buffer;

enum Session_ID : int32_t {
  regist_login_connect = -1,
  logout_disconnect = -2,
  accept_unread_msg = -3,
  user_noti = -4
};

netyi::netyi(std::string & dbpath) {
  YILOG_TRACE ("func: {}", __func__);

  std::string lpath = "netyi_" + dbpath;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, lpath, &db_);

  if (!status.ok()) {
    throw std::system_error(std::error_code(55000, 
          std::generic_category()),
       "open db failure");
  }

}

netyi::~netyi() {
  YILOG_TRACE ("func: {}", __func__);
  delete db_;
  clear_client();
}
/*
 * keys
 * */ 
// u_$userid
std::string user_key(const std::string & userid) {
  YILOG_TRACE ("func: {}", __func__);
  return "u_" + userid;
}
// p_$code_$phoneno
std::string user_phone_key(const std::string & countrycode,
    const std::string phoneno) {
  YILOG_TRACE ("func: {}", __func__);
  return "p_" + countrycode + "_" + phoneno;
}
// n_$tonodeid
std::string node_key(const std::string & tonodeid) {
  YILOG_TRACE ("func: {}", __func__);
  return "n_" + tonodeid;
}
// m_$tonodeid_$incrementid
std::string msg_key(const std::string & tonodeid,
    const std::string & incrementid) {
  YILOG_TRACE ("func: {}", __func__);
  return "m_" + tonodeid + "_" + incrementid;
}
std::string msg_key(const std::string & tonodeid,
    const int32_t incrementid) {
  return msg_key(tonodeid, std::to_string(incrementid));
}
// t_&userid
std::string talklist_key(const std::string & userid) {
  YILOG_TRACE ("func: {}", __func__);
  return "t_" + userid;
}
// e_userid_$nth [1, current]
std::string error_key(const int32_t nth);
  int32_t get_errorno();
  int32_t get_next_errorno_add();
/*
 * keys errno
 * */
std::string netyi::error_key(const int32_t  nth) {
  YILOG_TRACE ("func: {}", __func__);
  return "e_" + get_current_userid() + "_" + std::to_string(nth);
}
int32_t netyi::get_errorno() {
  YILOG_TRACE ("func: {}", __func__);
  auto key = error_key(0);
  std::string error_no_data;
  if (unlikely(get(key, error_no_data).ok())) {
    int32_t re = std::stoi(error_no_data);
    return re - 1;
  }else {
    return 0;
  }
}
int32_t netyi::get_next_errorno_add() {
  YILOG_TRACE ("func: {}", __func__);
  auto key = error_key(0);
  std::string error_no_data;
  if (unlikely(get(key, error_no_data).ok())) {
    int32_t re = std::stoi(error_no_data);
    error_no_data = std::to_string(re + 1);
    put(key, error_no_data);
    return re;
  }else {
    error_no_data = "2";
    put(key, error_no_data);
    return 1;
  }
}


/*
 * common
 * */
leveldb::Status netyi::put(const leveldb::Slice & key, 
    const leveldb::Slice & value) {
  YILOG_TRACE ("func: {}", __func__);
  return db_->Put(leveldb::WriteOptions(), key, value);
}
leveldb::Status netyi::get(const leveldb::Slice & key, 
    std::string & value) {
  YILOG_TRACE ("func: {}", __func__);
  return db_->Get(leveldb::ReadOptions(), key, &value);
}
/*
 * net func
 * */
void netyi::net_connect() {
  YILOG_TRACE ("func: {}", __func__);
  create_client([&](Buffer_SP sp){
    YILOG_TRACE ("net callback");
    switch(sp->datatype()) {
      case ChatType::registorres:
      case ChatType::loginres:
      case ChatType::clientconnectres:
      {
        auto key = signup_login_connect_key();
        call_map(Session_ID::regist_login_connect, key, sp);
        break;
      }
      case ChatType::loginnoti:
      case ChatType::addfriendnoti:
      case ChatType::addfriendauthorizenoti:
      {
        auto key = userinfo_noti_key();
        call_map(Session_ID::user_noti, key, sp);
        break;
      }
      case ChatType::logoutres:
      case ChatType::clientdisconnectres:
      {
        auto key = logout_disconnect_key();
        call_map(Session_ID::logout_disconnect, key, sp);
        break;
      }
      case ChatType::nodemessagenoti:
      {
        auto key = unread_noti_key();
        call_map(Session_ID::accept_unread_msg, key, sp);
        break;
      }
      default:
      call_map(sp->session_id(), sp);
    }

  });
}

// signup login connect
std::string netyi::signup_login_connect_key() {
  YILOG_TRACE ("func: {}", __func__);
  return "netyi_" "signup_login_connect_key";
}

// func(key, bool)
void netyi::signup_login_connect(Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map_send(Session_ID::regist_login_connect, sp,
      std::forward<CB_Func_Mutiple>(func));
}

// func(key, bool)
std::string netyi::logout_disconnect_key() {
  YILOG_TRACE ("func: {}", __func__);
  return "netyi_" "logout_disconnect_key";
}
void netyi::logout_disconnect(Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map_send(Session_ID::logout_disconnect, sp,
      std::forward<CB_Func_Mutiple>(func));
}
// func(sessionid, bool)
void netyi::send_buffer(Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map_send(sp,
      std::forward<CB_Func_Mutiple>(func));
}

/*
 * net noti
 *
 * */
std::string netyi::unread_noti_key() {
  YILOG_TRACE ("func: {}", __func__);
  return "netyi_" "unread_noti_key";
}
// unread msg noti func(key)
void netyi::acceptUnreadMsg(CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map(Session_ID::accept_unread_msg, 
      std::forward<CB_Func_Mutiple>(func));
}
// other device loginNoti addFriendNoti addFriendAuthorizeNoti
std::string netyi::userinfo_noti_key() {
  YILOG_TRACE ("func: {}", __func__);
  return "netyi_" "userinfo_noti_key";
}
// func (key)
void netyi::userInfoNoti(CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map(Session_ID::user_noti, 
      std::forward<CB_Func_Mutiple>(func));
}

/*
 * session id
 * */
std::string netyi::session_key(const int32_t sessionid, 
    const int32_t nth) {
  YILOG_TRACE ("func: {}", __func__);
  return "netyi_s_" + get_current_userid() + "_" + std::to_string(sessionid) 
    + "_" + std::to_string(nth);
}
int32_t netyi::get_sessionid_no(const int32_t sessionid) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = session_key(sessionid, 0);
  std::string session_no_data;
  if (unlikely(get(key, session_no_data).ok())) {
    int32_t re = std::stoi(session_no_data);
    return re - 1;
  }else {
    return 0;
  }
}
int32_t netyi::get_next_sessionid_no_add(const int32_t sessionid) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = session_key(sessionid, 0);
  std::string session_no_data;
  if (unlikely(get(key, session_no_data).ok())) {
    int32_t re = std::stoi(session_no_data);
    session_no_data = std::to_string(re + 1);
    put(key, session_no_data);
    return re;
  }else {
    session_no_data = "2";
    put(key, session_no_data);
    return 1;
  }
}
void netyi::clean_sessionid_keys(const int32_t sessionid) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = session_key(sessionid, 0);
  std::string session_no_data;
  if (likely(get(key, session_no_data).ok())) {
    int32_t nth = std::stoi(session_no_data);
    leveldb::WriteBatch batch;
    for (int32_t i = 0; i < nth; ++i) {
      batch.Delete(session_key(sessionid, i));
    }
    db_->Write(leveldb::WriteOptions(), &batch);
  }
}
/*
 * current 
 *
 * */
void netyi::set_current_userid(const std::string & userid) {
  YILOG_TRACE ("func: {}", __func__);
  if (likely(!put("current_user", userid).ok())) {
    throw std::system_error(std::error_code(55003,
          std::generic_category()),
        "set current user id failure");
  }
}
std::string netyi::get_current_userid() {
  YILOG_TRACE ("func: {}", __func__);
  std::string userid;
  if (likely(!get("current_user", userid).ok())) {
    throw std::system_error(std::error_code(55004,
          std::generic_category()),
        "get current user id failure");
  }
  return userid;
}

/*
 * private
 * */
void netyi::put_map(const int32_t sessionid, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  sessionid_cbfunc_map_[sessionid] = func;
}
void netyi::put_map_send(Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  uint16_t temp_session;
  client_send(sp, &temp_session);
  YILOG_TRACE ("func: {}, sessionid: {}", __func__, temp_session);
  sessionid_cbfunc_map_[temp_session] = func;
}
void netyi::put_map_send(const int32_t sessionid,
    Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  client_send(sp, nullptr);
  sessionid_cbfunc_map_[sessionid] = func;
}
bool netyi::call_map(const int32_t sessionid, Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  bool isCalled = false;
  auto lfunc = CB_Func_Mutiple();
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  auto it = sessionid_cbfunc_map_.find(sessionid);
  if (likely(it != sessionid_cbfunc_map_.end())) {
    lfunc = it->second;
  }else {
    YILOG_DEBUG ("user stop call back");
  }
  ul.unlock();
  if (lfunc) {
    bool isStop = true;
    auto key = std::to_string(get_sessionid_no(sessionid));
    put(key, leveldb::Slice(sp->data(), sp->data_size()));
    lfunc(key, sp->datatype(), &isStop);
    isCalled = true;
    if (isStop) {
      std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
      sessionid_cbfunc_map_.erase(it);
      clean_sessionid_keys(sessionid);
    }
  }
  return isCalled;
}

bool netyi::call_map(const int32_t sessionid, 
    std::string & key, Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  bool isCalled = false;
  auto lfunc = CB_Func_Mutiple();
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  auto it = sessionid_cbfunc_map_.find(sessionid);
  if (likely(it != sessionid_cbfunc_map_.end())) {
    lfunc = it->second;
  }else {
    YILOG_DEBUG ("user stop call back");
  }
  ul.unlock();
  if (lfunc) {
    bool isStop = true;
    put(key, leveldb::Slice(sp->data(), sp->data_size()));
    lfunc(key, sp->datatype(), &isStop);
    isCalled = true;

    std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
    sessionid_cbfunc_map_.erase(it);
  }
  return isCalled;
}

