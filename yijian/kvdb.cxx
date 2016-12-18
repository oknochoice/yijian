#include "kvdb.h"
#include <system_error>
#include <leveldb/write_batch.h>
#include "protofiles/chat_message.pb.h"

using yijian::buffer;

enum Session_ID : int32_t {
  regist_login_connect = -1,
  logout_disconnect = -2
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
std::string kvdb::errorKey(const std::string & userid,
    const std::string & nth) {
  YILOG_TRACE ("func: {}", __func__);
  return "e_" + userid + "_" + nth;
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
std::string kvdb::logoutKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "logout_kvdb";
}
std::string kvdb::connectKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "connect_kvdb";
}
std::string kvdb::disconnectKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "disconnect_kvdb";
}


/*
 * current 
 *
 * */
void kvdb::set_current_userid(const std::string & userid) {
  YILOG_TRACE ("func: {}", __func__);
  if (likely(!put("current_user", userid).ok())) {
    throw std::system_error(std::error_code(50003,
          std::generic_category()),
        "set current user id failure");
  }
}
std::string kvdb::get_current_userid() {
  YILOG_TRACE ("func: {}", __func__);
  std::string userid;
  if (likely(!get("current_user", userid).ok())) {
    throw std::system_error(std::error_code(50004,
          std::generic_category()),
        "get current user id failure");
  }
  return userid;
}
int32_t kvdb::get_current_error_maxth() {
  YILOG_TRACE ("func: {}", __func__);
  auto key = errorKey(get_current_userid(), 0);
  std::string error_data;
  if (get(key, error_data).ok()) {
    chat::ErrorNth nth;
    nth.ParseFromString(error_data);
    return nth.maxnth();
  }else {
    return 1;
  }
}
void kvdb::set_current_error(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  chat::ErrorNth nth;
  nth.set_maxnth(get_current_error_maxth() + 1);
  std::string key = errorKey(get_current_userid(),
      std::to_string(get_current_error_maxth()));
  leveldb::WriteBatch batch;
  leveldb::Slice error(sp->data(), sp->data_size());
  batch.Put(key, error);
  batch.Put(errorKey(get_current_userid(), 0), 
      nth.SerializeAsString());
  auto status = db_->Write(leveldb::WriteOptions(), &batch);
  if (!status.ok()) {
    throw std::system_error(std::error_code(50005,
          std::generic_category()),
        "set error msg failure");
  }
}

/*
 * user
 *
 * */
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
  auto userkey = userKey(userid);
  auto phonekey = userPhoneKey(countrycode, phoneno);
  leveldb::WriteBatch batch;
  batch.Put(phonekey, userid);
  batch.Put(userkey, user);
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


/*
 * busness regist login connect
 *
 * */ 
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
  device->set_devicemodel(devicemodel);
  device->set_uuid(uuid);
  client_send(buffer::Buffer(login), nullptr);
}
void kvdb::logout(const std::string & userid,
              const std::string & uuid,
              CB_Func && func) {
  // add func to map
  put_map(Session_ID::logout_disconnect, 
      std::forward<CB_Func>(func));
  // send logout
  chat::Logout logout;
  logout.set_userid(userid);
  logout.set_uuid(uuid);
  client_send(buffer::Buffer(logout), nullptr);
}
void kvdb::connect(const std::string & userid,
               const std::string & uuid,
               const bool isrecivenoti,
               const std::string & OSversion,
               const std::string & appversion,
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
  connect.set_osversion(OSversion);
  connect.set_clientversion(KVDBVersion);
  connect.set_appversion(appversion);
  client_send(buffer::Buffer(connect), nullptr);
}

void kvdb::disconnect(const std::string & userid,
               const std::string & uuid,
               CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  // add func to map
  put_map(Session_ID::logout_disconnect, 
      std::forward<CB_Func>(func));
  // send connect
  chat::ClientDisConnect disconnect;
  disconnect.set_userid(userid);
  disconnect.set_uuid(uuid);
  client_send(buffer::Buffer(disconnect), nullptr);
}

/*
 * busness add friend
 *
 * */
void kvdb::addfriend(const std::string & inviteeid,
                 const std::string & msg,
                 CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::AddFriend add;
  add.set_inviterid(get_current_userid());
  add.set_inviteeid(inviteeid);
  add.set_msg(msg);
  put_map_send(std::forward<CB_Func>(func), buffer::Buffer(add));
}

void kvdb::addfriendAuthorize(const std::string & inviterid,
                        const std::string & tonodeid,
                        int isAgree,
                        CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::AddFriendAuthorize authorize;
  authorize.set_inviterid(inviterid);
  authorize.set_inviteeid(get_current_userid());
  authorize.set_isagree(static_cast<chat::IsAgree>(isAgree));
  authorize.set_tonodeid(tonodeid);
  put_map_send(std::forward<CB_Func>(func), 
      buffer::Buffer(authorize));
}




// private

void kvdb::put_map(int32_t sessionid, CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  sessionid_cbfunc_map_[sessionid] = func;
}
void kvdb::put_map_send(CB_Func && func, Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  client_send(sp, &temp_session_);
  sessionid_cbfunc_map_[temp_session_] = func;
}

void kvdb::call_erase_map(int32_t sessionid, std::string & key) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  auto it = sessionid_cbfunc_map_.find(sessionid);
  if (likely(it != sessionid_cbfunc_map_.end())) {
    it->second(key);
    sessionid_cbfunc_map_.erase(sessionid);
  }else {
    throw std::system_error(std::error_code(50020, 
          std::generic_category()),
        "sessionid_cbfunc_map_ not find type"); 
  }
}


void kvdb::dispatch(int type, Buffer_SP sp) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto static map_p = new std::map<int, std::function<void(void)>>();
  std::once_flag flag;
  std::call_once(flag, [=]() {
      // regist login logout connect disconnet
      (*map_p)[ChatType::registorres] = [=]() {
        registerRes(sp);
      };
      (*map_p)[ChatType::loginres] = [=]() {
        loginRes(sp);
      };
      (*map_p)[ChatType::logoutres] = [=]() {
        logoutRes(sp);
      };
      (*map_p)[ChatType::clientconnectres] = [=]() {
        connectRes(sp);
      };
      (*map_p)[ChatType::clientdisconnectres] = [=]() {
        disconnectRes(sp);
      };
      // error
      (*map_p)[ChatType::error] = [=]() {
        error(sp);
      };
      // other
      (*map_p)[ChatType::addfriendres] = [=]() {
        addfriendRes(sp);
      };
      (*map_p)[ChatType::addfriendauthorizeres] = [=]() {
        addfriendAuthorizeRes(sp);
      };
  });
  auto it = map_p->find(type);
  if (it != map_p->end()) {
    it->second();
  }else {
    throw std::system_error(std::error_code(50021, 
          std::generic_category()),
        "map_p not find type"); 
  }
}

// regist login logout connect disconnet
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

void kvdb::logoutRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto value = leveldb::Slice(sp->data(), sp->data_size());
  auto key = logoutKey();
  put(key, value);
  call_erase_map(Session_ID::logout_disconnect, key);
}

void kvdb::connectRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto value = leveldb::Slice(sp->data(), sp->data_size());
  auto key = connectKey();
  put(key, value);
  call_erase_map(Session_ID::regist_login_connect, key);
}

void kvdb::disconnectRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto value = leveldb::Slice(sp->data(), sp->data_size());
  auto key = disconnectKey();
  put(key, value);
  call_erase_map(Session_ID::logout_disconnect, key);
}

void kvdb::error(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  set_current_error(sp);
  std::string key = errorKey(get_current_userid(),
      std::to_string(get_current_error_maxth() - 1));
  call_erase_map(sp->session_id(), key);
}

void kvdb::addfriendRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = userKey(get_current_userid());
  call_erase_map(sp->session_id(), key);
}
void kvdb::addfriendAuthorizeRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = userKey(get_current_userid());
  call_erase_map(sp->session_id(), key);
}


