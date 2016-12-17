#include "kvdb.h"
#include <system_error>
#include <leveldb/write_batch.h>

using yijian::buffer;

enum Session_ID : int32_t {
  regist_login_connect = -1
};

kvdb::kvdb(std::string & path) {
  YILOG_TRACE ("func: {}", __func__);
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, path, &db_);

  if (!status.ok()) {
    throw std::system_error(std::error_code(50000, 
          std::generic_category()),
       "open db failure");
  }

  create_client([&](Buffer_SP sp){
    YILOG_TRACE ("net callback");
    dispatch(sp->datatype(), sp);
  });
}

kvdb::~kvdb() {
  YILOG_TRACE ("func: {}", __func__);
  delete db_;
}

// common
leveldb::Status kvdb::put(const leveldb::Slice & key, 
    const leveldb::Slice & value) {
  YILOG_TRACE ("func: {}", __func__);
  return db_->Put(leveldb::WriteOptions(), key, value);
}
leveldb::Status kvdb::get(const leveldb::Slice & key, 
    std::string & value) {
  YILOG_TRACE ("func: {}", __func__);
  return db_->Get(leveldb::ReadOptions(), key, &value);
}

std::string kvdb::userKey(const std::string & userid) {
  YILOG_TRACE ("func: {}", __func__);
  return "u_" + userid;
}

std::string kvdb::msgKey(const std::string & tonodeid,
    const std::string & incrementid) {
  YILOG_TRACE ("func: {}", __func__);
  return "m_" + tonodeid + "_" + incrementid;
}
std::string kvdb::userPhoneKey(const std::string & countrycode,
    const std::string phoneno) {
  YILOG_TRACE ("func: {}", __func__);
  return "p_" + countrycode + "_" + phoneno;
}
std::string kvdb::errorKey(const std::string & nth) {
  YILOG_TRACE ("func: {}", __func__);
  return "e_" + nth;
}
std::string kvdb::talklistKey(const std::string & userid) {
  YILOG_TRACE ("func: {}", __func__);
  return "t_" + userid;
}
std::string kvdb::signupKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "signup_kvdb";
}
std::string kvdb::loginKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "login_kvdb";
}
std::string kvdb::connectKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "connect_kvdb";
}

// user
leveldb::Status kvdb::putUser(const std::string & userid, 
      const std::string & countrycode,
      const std::string & phoneno,
    const leveldb::Slice & user) {
  YILOG_TRACE ("func: {}", __func__);
  if (userid.empty() || countrycode.empty() || phoneno.empty()) {
    throw std::system_error(std::error_code(50001,
          std::generic_category()),
        "parameter is empty");
  }
  auto useridkey = userKey(userid);
  auto phonekey = userPhoneKey(countrycode, phoneno);
  leveldb::WriteBatch batch;
  batch.Put(phonekey, userid);
  batch.Put(useridkey, user);
  return db_->Write(leveldb::WriteOptions(), &batch);
}

leveldb::Status kvdb::getUser(const std::string & id, 
    std::string & user) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = userKey(id);
  return get(key, user);
}

leveldb::Status kvdb::getUser(const std::string & countrycode,
    const std::string & phoneno,
    std::string & user) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = userPhoneKey(countrycode, phoneno);
  std::string userid;
  get(key, userid);
  return getUser(userid, user);
}

// message
leveldb::Status kvdb::putTalklist(const std::string & userid, 
    const leveldb::Slice & talklist) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = talklistKey(userid);
  return put(key, talklist);
}
leveldb::Status kvdb::getTalklist(const std::string & userid, 
    std::string & talklist) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = talklistKey(userid);
  return get(key, talklist);
}
leveldb::Status kvdb::putMsg(const std::string & msgNode,
    const int32_t & incrementid,
    const leveldb::Slice & msg) {
  YILOG_TRACE ("func: {}", __func__);
  if (msgNode.empty() || 0 == incrementid) {
    throw std::system_error(std::error_code(50002,
          std::generic_category()),
        "msg miss tonodeid or incrementid");
  }
  auto key = msgKey(msgNode, std::to_string(incrementid));
  return put(key, msg);
}
leveldb::Status kvdb::getMsg(const std::string & msgNode,
    const int32_t & incrementid,
    std::string & msg) {
  YILOG_TRACE ("func: {}", __func__);

  if (msgNode.empty() || 0 >= incrementid) {
    throw std::system_error(std::error_code(50003,
          std::generic_category()),
        "msg miss tonodeid or incrementid");
  }

  std::string value;
  auto key = msgKey(msgNode, std::to_string(incrementid));
  return get(key, msg);

}


// busness
void kvdb::registUser(const std::string & phoneno,
                  const std::string & countrycode,
                  const std::string & password,
                  const std::string & verifycode,
                  const std::string & nickname,
                  CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  // check
#warning phone num verify
  if (unlikely(phoneno.empty())) {
    throw std::system_error(std::error_code(50010,
          std::generic_category()),
        "phoneno is not valid.");
  }
#warning country code verify
  if (unlikely(countrycode.empty())) {
    throw std::system_error(std::error_code(50011,
          std::generic_category()),
        "countrycode is not valid.");
  }
  if (unlikely(password.size() < 6)) {
    throw std::system_error(std::error_code(50012,
          std::generic_category()),
        "password length at least 6");
  }
  if (unlikely(verifycode.empty())) {
    throw std::system_error(std::error_code(50013,
          std::generic_category()),
        "verifycode is no valid");
  }
  if (unlikely(nickname.empty())) {
    throw std::system_error(std::error_code(50014,
          std::generic_category()),
        "nickname is empty");
  }
  // add func to map
  put_map(Session_ID::regist_login_connect, 
      std::forward<CB_Func>(func));
  // send regist
  chat::Register enroll;
  enroll.set_phoneno(phoneno);
  enroll.set_countrycode(countrycode);
#warning need encrypt
  enroll.set_password(password);
  enroll.set_nickname(nickname);
  enroll.set_verifycode(verifycode);
  client_send(buffer::Buffer(enroll), nullptr);

}

void kvdb::login(const std::string & phoneno,
             const std::string & countrycode,
             const std::string & password,
             const int os,
             const std::string & OSversion,
             const std::string & appversion,
             const std::string & devicemodel,
             const std::string & uuid,
             CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  // add func to map
  put_map(Session_ID::regist_login_connect, 
      std::forward<CB_Func>(func));
  // send login
  chat::Login login;
  login.set_phoneno(phoneno);
  login.set_countrycode(countrycode);
#warning need encrypt
  login.set_password(password);
  auto device = login.mutable_device();
  device->set_os(static_cast<chat::Device::OperatingSystem>(os));
  device->set_osversion(OSversion);
  device->set_clientversion(KVDBVersion);
  device->set_appversion(appversion);
  device->set_devicemodel(devicemodel);
  device->set_uuid(uuid);
  client_send(buffer::Buffer(login), nullptr);
}
void kvdb::connect(const std::string & userid,
               const std::string & uuid,
               const bool isrecivenoti,
               CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  // add func to map
  put_map(Session_ID::regist_login_connect, 
      std::forward<CB_Func>(func));
  // send connect
  chat::ClientConnect connect;
  connect.set_userid(userid);
  connect.set_uuid(uuid);
  connect.set_isrecivenoti(isrecivenoti);
  client_send(buffer::Buffer(connect), nullptr);
}

// private

void kvdb::put_map(int32_t sessionid, CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  sessionid_cbfunc_map_[sessionid] = func;
}

void kvdb::call_erase_map(int32_t sessionid, std::string & key) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  sessionid_cbfunc_map_.at(sessionid)(key);
  sessionid_cbfunc_map_.erase(sessionid);
}


void kvdb::dispatch(int type, Buffer_SP sp) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto static map_p = new std::map<int, std::function<void(void)>>();
  std::once_flag flag;
  std::call_once(flag, [=]() {
      (*map_p)[ChatType::registorres] = [=]() {
        registerRes(sp);
      };
      (*map_p)[ChatType::loginres] = [=]() {
        loginRes(sp);
      };
      (*map_p)[ChatType::clientconnectres] = [=]() {
        connectRes(sp);
      };
  });
  (*map_p)[type]();
}

// 
void kvdb::registerRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto value = leveldb::Slice(sp->data(), sp->data_size());
  auto key = signupKey();
  put(key, value);
  call_erase_map(Session_ID::regist_login_connect, key);
}

void kvdb::loginRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto value = leveldb::Slice(sp->data(), sp->data_size());
  auto key = loginKey();
  put(key, value);
  call_erase_map(Session_ID::regist_login_connect, key);
}

void kvdb::connectRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  chat::ClientConnectRes res;
  res.ParseFromArray(sp->data(), sp->data_size());
  sessionid_ = res.sessionid();
  auto value = leveldb::Slice(sp->data(), sp->data_size());
  auto key = connectKey();
  put(key, value);
  call_erase_map(Session_ID::regist_login_connect, key);
}

