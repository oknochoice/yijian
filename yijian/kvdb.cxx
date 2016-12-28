#include "kvdb.h"
#include <system_error>
#include <leveldb/write_batch.h>
#include "protofiles/chat_message.pb.h"
#include <openssl/sha.h>

using yijian::buffer;

enum Session_ID : int32_t {
  regist_login_connect = -1,
  logout_disconnect = -2,
  accept_unread_msg = -3,
  user_noti = -4
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
std::string kvdb::nodeKey(const std::string & tonodeid) {
  YILOG_TRACE ("func: {}", __func__);
  return "n_" + tonodeid;
}
std::string kvdb::msgKey(const std::string & tonodeid,
    const std::string & incrementid) {
  YILOG_TRACE ("func: {}", __func__);
  return "m_" + tonodeid + "_" + incrementid;
}
std::string kvdb::msgKey(const std::string & tonodeid,
    const int32_t incrementid) {
  return msgKey(tonodeid, std::to_string(incrementid));
}
std::string kvdb::userPhoneKey(const std::string & countrycode,
    const std::string phoneno) {
  YILOG_TRACE ("func: {}", __func__);
  return "p_" + countrycode + "_" + phoneno;
}
std::string kvdb::errorKey(const std::string & userid,
    const int32_t  nth) {
  YILOG_TRACE ("func: {}", __func__);
  return "e_" + userid + "_" + std::to_string(nth);
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
std::string kvdb::loginNotiKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "loginnoti_kvdb";
}
std::string kvdb::addFriendNotiKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "addfriendnoti_kvdb";
}
std::string kvdb::addFriendAuthorizeNotiKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "addfriendauthorizenoti_kvdb";
}
std::string kvdb::addFriendInfoKey() {
  YILOG_TRACE ("func: {}", __func__);
  return "addfriendinfo_kvdb";
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
int32_t kvdb::get_errorno() {
  YILOG_TRACE ("func: {}", __func__);
  auto key = errorKey(get_current_userid(), 0);
  std::string error_data;
  if (get(key, error_data).ok()) {
    chat::ErrorNth nth;
    nth.ParseFromString(error_data);
    int32_t re;
    re = nth.maxnth();
    nth.set_maxnth(re + 1);
    put(key, nth.SerializeAsString());
    return re;
  }else {
    chat::ErrorNth nth;
    nth.set_maxnth(2);
    put(key, nth.SerializeAsString());
    return 1;
  }
}

/*
 * user
 *
 * */
void kvdb::putUser(const std::string & userid, 
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
  auto status = db_->Write(leveldb::WriteOptions(), &batch);
  if (unlikely(!status.ok())) {
    throw std::system_error(std::error_code(50006,
          std::generic_category()),
        "put user failure");
  }
}

bool kvdb::getUser(const std::string & id, 
    std::string & user) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = userKey(id);
  auto status = get(key, user);
  if (unlikely(!status.ok())) {
    /*
    throw std::system_error(std::error_code(50007,
          std::generic_category()),
        "get user failure");
        */
    return false;
  }
  return true;
}

bool kvdb::getUser(const std::string & countrycode,
    const std::string & phoneno,
    std::string & user) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = userPhoneKey(countrycode, phoneno);
  std::string userid;
  get(key, userid);
  return getUser(userid, user);
}

// message
void kvdb::putTalklist(const std::string & userid, 
    const leveldb::Slice & talklist) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = talklistKey(userid);
  auto status = put(key, talklist);
  if (unlikely(!status.ok())) {
    throw std::system_error(std::error_code(50008,
          std::generic_category()),
        "put talklist failure");
  }
}
bool kvdb::getTalklist(const std::string & userid, 
    std::string & talklist) {
  YILOG_TRACE ("func: {}", __func__);
  auto key = talklistKey(userid);
  auto status = get(key, talklist);
  if (unlikely(!status.ok())) {
    /*
    throw std::system_error(std::error_code(50009,
          std::generic_category()),
        "get talklist failure");
        */
    return false;
  }
  return true;
}
void kvdb::putMsg(const std::string & msgNode,
    const int32_t & incrementid,
    const leveldb::Slice & msg) {
  YILOG_TRACE ("func: {}", __func__);
  if (msgNode.empty() || 0 == incrementid) {
    throw std::system_error(std::error_code(50002,
          std::generic_category()),
        "msg miss tonodeid or incrementid");
  }
  auto key = msgKey(msgNode, incrementid);
  auto status = put(key, msg);
  if (unlikely(!status.ok())) {
    throw std::system_error(std::error_code(50040,
          std::generic_category()),
        "put message failure");
  }
}
bool kvdb::getMsg(const std::string & msgNode,
    const int32_t & incrementid,
    std::string & msg) {
  YILOG_TRACE ("func: {}", __func__);

  if (msgNode.empty() || 0 >= incrementid) {
    throw std::system_error(std::error_code(50003,
          std::generic_category()),
        "msg miss tonodeid or incrementid");
  }

  std::string value;
  auto key = msgKey(msgNode, incrementid);
  auto status = get(key, msg);
  if (unlikely(!status.ok())) {
    /*
    throw std::system_error(std::error_code(50041,
          std::generic_category()),
        "get message failure");
        */
    return false;
  }
  return true;

}


/*
 * network regist login connect
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
  device->set_devicenickname("");
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
 * network friend
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
  put_map_send(buffer::Buffer(add), std::forward<CB_Func>(func));
}

void kvdb::addfriendAuthorize(const std::string & inviterid,
                        int isAgree,
                        CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::AddFriendAuthorize authorize;
  authorize.set_inviterid(inviterid);
  authorize.set_inviteeid(get_current_userid());
  authorize.set_isagree(static_cast<chat::IsAgree>(isAgree));
  put_map_send(buffer::Buffer(authorize), 
      std::forward<CB_Func>(func));
}

void kvdb::queryaddfriendinfo(CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  db_->Delete(leveldb::WriteOptions(), addFriendInfoKey());
  chat::QueryAddfriendInfo query;
  query.set_count(10);
  put_map_send(buffer::Buffer(query),
      std::forward<CB_Func>(func));
}

/*
 * network group
 *
 * */
void kvdb::creategroup(const std::string & groupname,
    const std::vector<std::string> membersid,
    CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::CreateGroup group;
  group.set_userid(get_current_userid());
  group.set_nickname(groupname);
  for (auto & memberid: membersid) {
    group.add_membersid(memberid);
  }
  put_map_send(buffer::Buffer(group), 
      std::forward<CB_Func>(func));
}
void kvdb::addmembers2group(const std::string & groupid,
    const std::vector<std::string> membersid,
    CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::GroupAddMember add;
  add.set_tonodeid(groupid);
  for (auto & memberid: membersid) {
    add.add_membersid(memberid);
  }
  put_map_send(buffer::Buffer(add), 
      std::forward<CB_Func>(func));
}

/*
 * network user
 *
 * */
void kvdb::queryuserVersion(const std::string & userid,
    CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::QueryUserVersion query;
  query.set_userid(userid);
  put_map_send(buffer::Buffer(query), 
      std::forward<CB_Func>(func));
}
void kvdb::queryuserVersion(CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  queryuserVersion(get_current_userid(), 
      std::forward<CB_Func>(func));
}
void kvdb::queryuser(const std::string && userid, 
    CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::QueryUser query;
  query.set_userid(userid);
  put_map_send(buffer::Buffer(query), 
      std::forward<CB_Func>(func));
}
void kvdb::queryuser(CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  queryuser(get_current_userid(), 
      std::forward<CB_Func>(func));
}

/*
 * network node and message
 *
 * func
 * failure error key
 *
 * */
void kvdb::querynodeversion(const std::string & nodeid,
                        CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::QueryNodeVersion query;
  query.set_tonodeid(nodeid);
  put_map_send(buffer::Buffer(query), 
      std::forward<CB_Func>(func));
}
void kvdb::querynode(const std::string & nodeid, 
    CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::QueryNode query;
  query.set_tonodeid(nodeid);
  put_map_send(buffer::Buffer(query), 
      std::forward<CB_Func>(func));
}
void kvdb::sendmessage2user(const std::string & userid,
                        const std::string & tonodeid,
                        const int type,
                        const std::string & contenct,
                        CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::NodeMessage msg;
  msg.set_touserid_outer(userid);
  msg.set_tonodeid(tonodeid);
  msg.set_type(static_cast<chat::MediaType>(type));
  msg.set_content(contenct);
  put_map_send_dbcache(buffer::Buffer(msg), 
      std::forward<CB_Func>(func));

}
void kvdb::sendmessage2group(const std::string & tonodeid,
                      const int type,
                      const std::string & contenct,
                      CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::NodeMessage msg;
  msg.set_tonodeid(tonodeid);
  msg.set_type(static_cast<chat::MediaType>(type));
  msg.set_content(contenct);
  put_map_send_dbcache(buffer::Buffer(msg), 
      std::forward<CB_Func>(func));
}
void kvdb::querymsg(const std::string & tonodeid,
              const int32_t fromincrementid,
              const int32_t toincrementid,
              CB_Func_Mutiple && mfunc) {
  YILOG_TRACE ("func: {}", __func__);
  chat::QueryMessage query;
  query.set_tonodeid(tonodeid);
  query.set_fromincrementid(fromincrementid);
  query.set_toincrementid(toincrementid);
  put_mmap_send(buffer::Buffer(query),
      std::forward<CB_Func_Mutiple>(mfunc));
}

void kvdb::queryonemsg(const std::string & tonodeid,
                   const int32_t incrementid,
                   CB_Func_Mutiple && mfunc) {
  YILOG_TRACE ("func: {}", __func__);
  chat::QueryOneMessage query;
  query.set_tonodeid(tonodeid);
  query.set_incrementid(incrementid);
  put_mmap_send(buffer::Buffer(query),
      std::forward<CB_Func_Mutiple>(mfunc));
}

// type MediaType in proto
void kvdb::sendmedia(const int type,
               const std::string & content,
               CB_Func && process) {
  YILOG_TRACE ("func: {}", __func__);


  int i = 0;
  std::size_t pos = 0;
  unsigned char sha1_result[SHA_DIGEST_LENGTH];
  SHA_CTX ctx;
  SHA1_Init(&ctx);
  SHA1_Update(&ctx, content.data(), content.size());
  SHA1_Final(sha1_result, &ctx);
  std::string sha1(reinterpret_cast<char *>(sha1_result), 
      SHA_DIGEST_LENGTH);
  put(sha1, content);
  put_media_map(sha1, std::forward<CB_Func>(process));
  int32_t maxLength = static_cast<int32_t>(Message_Type::message)
                      - PADDING_LENGTH;
  do {
    chat::Media media;
    media.set_sha1(sha1);
    media.set_nth(i);
    media.set_length(content.size());
    media.set_type(static_cast<chat::MediaType>(type));
    auto length = maxLength - media.ByteSize();
    auto subContent = content.substr(pos, length);
    ++i;
    pos += length;
    media.set_content(subContent);
    sendmedia(media, [sha1, length, this](const std::string & key){
          std::string key_length = key + "_" + std::to_string(length);
          if (key < std::to_string(length)) {
            call_media_map(sha1, key_length);
          }else if (key == std::to_string(length)) {
            callerase_media_map(sha1, key_length);
          }else {
            throw std::system_error(std::error_code(50015,
                  std::generic_category()),
                "media length error");
          }
        });
  }while(pos >= content.size());
}

void kvdb::sendmedia(chat::Media & media,
               CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map_send(buffer::Buffer(media),
      std::forward<CB_Func>(func));
}

void kvdb::querymedia(const std::string & sha1,
                  CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::QueryMedia query;
  query.set_sha1(sha1);
  put_media_map(sha1, std::forward<CB_Func>(func));
  client_send(buffer::Buffer(query), nullptr);
}

void kvdb::sendmediaIsexist(const std::string & content,
                        CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  chat::MediaIsExist query;

  unsigned char sha1_result[SHA_DIGEST_LENGTH];
  SHA_CTX ctx;
  SHA1_Init(&ctx);
  SHA1_Update(&ctx, content.data(), content.size());
  SHA1_Final(sha1_result, &ctx);
  std::string sha1(reinterpret_cast<char *>(sha1_result), 
      SHA_DIGEST_LENGTH);
  query.set_sha1(sha1);
  put_map_send(buffer::Buffer(query), 
      std::forward<CB_Func>(func));
}

void kvdb::sendmediacheck(const std::string & content,
                      CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  unsigned char sha1_result[SHA_DIGEST_LENGTH];
  SHA_CTX ctx;
  SHA1_Init(&ctx);
  SHA1_Update(&ctx, content.data(), content.size());
  SHA1_Final(sha1_result, &ctx);
  std::string sha1(reinterpret_cast<char *>(sha1_result), 
      SHA_DIGEST_LENGTH);
  chat::MediaCheck check;
  check.set_sha1(sha1);
  put_map_send(buffer::Buffer(check), 
      std::forward<CB_Func>(func));
}

void kvdb::acceptUnreadMsg(CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map(Session_ID::accept_unread_msg, 
      std::forward<CB_Func>(func));
}

void kvdb::userInfoNoti(CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map(Session_ID::user_noti, 
      std::forward<CB_Func>(func));
}

/*
 *
 * private
 *
 * */
void kvdb::put_map(const int32_t sessionid, CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  sessionid_cbfunc_map_[sessionid] = func;
}
void kvdb::call_map(const int32_t sessionid, 
    const std::string & key) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  auto it = sessionid_cbfunc_map_.find(sessionid);
  if (likely(it != sessionid_cbfunc_map_.end())) {
    it->second(key);
  }
}
void kvdb::put_map_send(Buffer_SP sp, CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  uint16_t temp_session;
  client_send(sp, &temp_session);
  YILOG_TRACE ("func: {}, sessionid: {}", __func__, temp_session);
  sessionid_cbfunc_map_[temp_session] = func;
}
void kvdb::call_erase_map(const int32_t sessionid, 
    const std::string & key) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  auto it = sessionid_cbfunc_map_.find(sessionid);
  if (likely(it != sessionid_cbfunc_map_.end())) {
    it->second(key);
    sessionid_cbfunc_map_.erase(sessionid);
  }else {
    YILOG_CRITICAL ("errno:50020, sessionid {} not find in"
        " sessionid_cbfunc_map_ .", sessionid);
    /*
    throw std::system_error(std::error_code(50020, 
          std::generic_category()),
        "sessionid_cbfunc_map_ not find type"); 
        */
  }
}

bool kvdb::maycall_erase_map(
    const int32_t sessionid, const std::string & key) {
  YILOG_TRACE ("func: {}", __func__);
  bool isCalled = false;
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  auto it = sessionid_cbfunc_map_.find(sessionid);
  if (likely(it != sessionid_cbfunc_map_.end())) {
    it->second(key);
    sessionid_cbfunc_map_.erase(sessionid);
    isCalled = true;
  }else {
    isCalled = false;
  }
  return isCalled;
}

void kvdb::put_map_send_dbcache(Buffer_SP sp, CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  uint16_t temp_session;
  client_send(sp, &temp_session);
  YILOG_TRACE ("func: {}, sessionid: {}", __func__, temp_session);
  sessionid_cbfunc_map_[temp_session] = func;
  auto key = std::to_string(temp_session);
  auto value = leveldb::Slice(sp->data(), sp->data_size());
  put(key, value);
}
void kvdb::put_media_map(const std::string & sha1, CB_Func && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(media_map_mutex_);
  media_cbfunc_map_[sha1] = func;
}

void kvdb::call_media_map(const std::string & sha1, 
                      const std::string & key) {
  YILOG_TRACE ("func: {}. ", __func__);
  std::unique_lock<std::mutex> ul(media_map_mutex_);
  auto it = media_cbfunc_map_.find(sha1);
  if (likely( it != media_cbfunc_map_.end())) {
    it->second(key);
  }else {
    YILOG_CRITICAL ("error no: 500021, sha1 {} "
        "not find in media_cbfunc_map_", sha1);
    /*
    throw std::system_error(std::error_code(50021, 
          std::generic_category()),
        "media_cbfunc_map_ not find type"); 
        */
  }
}

void kvdb::callerase_media_map(const std::string & sha1,
    const std::string & key) {
  YILOG_TRACE ("func: {}. ", __func__);
  std::unique_lock<std::mutex> ul(media_map_mutex_);
  auto it = media_cbfunc_map_.find(sha1);
  if (likely( it != media_cbfunc_map_.end())) {
    it->second(key);
    media_cbfunc_map_.erase(it);
  }else {
    YILOG_CRITICAL ("error no: 500022, sha1 {} "
        "not find in media_cbfunc_map_", sha1);
    /*
    throw std::system_error(std::error_code(50022, 
          std::generic_category()),
        "media_cbfunc_map_ not find type"); 
        */
  }
}


void kvdb::put_mmap_send(Buffer_SP sp, CB_Func_Mutiple && mfunc) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_mmap_mutex_);
  uint16_t temp_session;
  client_send(sp, &temp_session);
  YILOG_TRACE ("func: {}, sessionid: {}", __func__, temp_session);
  sessionid_mmap_[temp_session] = mfunc;
}
void kvdb::call_mmap(const int32_t sessionid, 
    const std::string & key) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_mmap_mutex_);
  auto it = sessionid_mmap_.find(sessionid);
  if (likely(it != sessionid_mmap_.end())) {
    bool isStop = true;
    it->second(key, &isStop);
    if (isStop) {
      sessionid_mmap_.erase(it);
    }
  }else {
    YILOG_DEBUG ("user stop call back");
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
      // noti
      (*map_p)[ChatType::nodemessagenoti] = [=]() {
        msgunreadnoti(sp);
      };
      (*map_p)[ChatType::loginnoti] = [=]() {
        usernoti(sp);
      };
      (*map_p)[ChatType::addfriendnoti] = [=]() {
        usernoti(sp);
      };
      (*map_p)[ChatType::addfriendauthorizenoti] = [=]() {
        usernoti(sp);
      };
      // friend group
      (*map_p)[ChatType::addfriendres] = [=]() {
        addfriendRes(sp);
      };
      (*map_p)[ChatType::addfriendauthorizeres] = [=]() {
        addfriendAuthorizeRes(sp);
      };
      (*map_p)[ChatType::queryaddfriendinfores] = [=]() {
        queryaddfriendinfoRes(sp);
      };
      (*map_p)[ChatType::creategroupres] = [=]() {
        creategroupRes(sp);
      };
      (*map_p)[ChatType::groupaddmemberres] = [=]() {
        groupaddmemberRes(sp);
      };
      // user
      (*map_p)[ChatType::queryuserres] = [=]() {
        queryuserRes(sp);
      };
      (*map_p)[ChatType::queryuserversionres] = [=]() {
        queryuserversionRes(sp);
      };
      // node and message
      (*map_p)[ChatType::querynodeversionres] = [=]() {
        querynodeversionRes(sp);
      };
      (*map_p)[ChatType::querynoderes] = [=]() {
        querynodeRes(sp);
      };
      (*map_p)[ChatType::nodemessageres] = [=]() {
        messageRes(sp);
      };
      (*map_p)[ChatType::nodemessage] = [=]() {
        nodemessage(sp);
      };
      // media
      (*map_p)[ChatType::mediares] = [=]() {
        mediaRes(sp);
      };
      (*map_p)[ChatType::media] = [=]() {
        media(sp);
      };
      (*map_p)[ChatType::mediaisexistres] = [=]() {
        mediaisexistRes(sp);
      };
      (*map_p)[ChatType::mediacheckres] = [=]() {
        mediacheckRes(sp);
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
  chat::LoginRes res;
  res.ParseFromArray(sp->data(), sp->data_size());
  if (res.issuccess()) {
    set_current_userid(res.userid());
  }
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
  std::string key = errorKey(get_current_userid(),
      get_errorno());
  auto value = leveldb::Slice(sp->data(), sp->data_size());
  chat::Error err;
  err.ParseFromArray(sp->data(), sp->data_size());
  YILOG_TRACE ("errno: {}, errmsg {}.", err.errnum(), err.errmsg());
  put(key, value);
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
void kvdb::queryaddfriendinfoRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  // res
  chat::QueryAddfriendInfoRes res;
  res.ParseFromArray(sp->data(), sp->data_size());
  if (res.isend() == false) {
    // info
    std::string value;
    chat::AddFriendInfo info;
    if (get(addFriendInfoKey(), value).ok()) {
      info.ParseFromString(value);
    }
    auto it = std::find_if(info.info().begin(), info.info().end(),
        [&res](const chat::QueryAddfriendInfoRes & a){
          return a.inviter() == res.inviter() &&
                 a.invitee() == res.invitee();
        });
    if (it != info.info().end()) {
      auto newInfo = info.mutable_info()->Add();
      *newInfo = res;
    }
    put(addFriendInfoKey(), info.SerializeAsString());
  }else {
    call_erase_map(sp->session_id(), addFriendInfoKey());
  }
}
void kvdb::creategroupRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto node = chat::CreateGroupRes();
  node.ParseFromArray(sp->data(), sp->data_size());
  auto key = nodeKey(node.tonodeid());
  call_erase_map(sp->session_id(), key);
}
void kvdb::groupaddmemberRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto node = chat::GroupAddMemberRes();
  node.ParseFromArray(sp->data(), sp->data_size());
  auto key = nodeKey(node.tonodeid());
  call_erase_map(sp->session_id(), key);
}
void kvdb::queryuserversionRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto res = chat::QueryUserVersionRes();
  res.ParseFromArray(sp->data(), sp->data_size());
  auto key = std::to_string(res.version());
  call_erase_map(sp->session_id(), key);
}
void kvdb::queryuserRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto res = chat::QueryUserRes();
  res.ParseFromArray(sp->data(), sp->data_size());
  auto user = chat::User();
  user = res.user();
  auto key = userKey(user.id());
  put(key, user.SerializeAsString());
  call_erase_map(sp->session_id(), key);
}
void kvdb::querynodeversionRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto res = chat::QueryNodeVersionRes();
  res.ParseFromArray(sp->data(), sp->data_size());
  auto version = std::to_string(res.version());
  call_erase_map(sp->session_id(), version);
}
void kvdb::querynodeRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  auto res = chat::QueryNodeRes();
  res.ParseFromArray(sp->data(), sp->data_size());
  auto key = nodeKey(res.node().id());
  auto value = leveldb::Slice(sp->data(), sp->data_size());
  put(key, value);
  call_erase_map(sp->session_id(), key);
}
void kvdb::messageRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  std::string msg_string;
  if (get(std::to_string(sp->session_id()), msg_string).ok()) {
    // update msg 
    auto res = chat::NodeMessageRes();
    res.ParseFromArray(sp->data(), sp->data_size());
    auto msg = chat::NodeMessage();
    msg.ParseFromString(msg_string);
    msg.set_incrementid(res.incrementid());
    // compose ke value
    std::string key = msgKey(msg.tonodeid(), 
        msg.incrementid());
    leveldb::Slice value(msg.SerializeAsString());
    // update database
    leveldb::WriteBatch batch;
    batch.Delete(std::to_string(sp->session_id()));
    batch.Put(key, value);
    auto status = db_->Write(leveldb::WriteOptions(), &batch);
    if (!status.ok()) {
      throw std::system_error(std::error_code(50031, 
            std::generic_category()),
         "delete temporary key (sessionid) and insert msg failure");
    }
    call_erase_map(sp->session_id(), key);
  }else {
    throw std::system_error(std::error_code(50030, 
          std::generic_category()),
       "not find temporary key (sessionid)");
  }
}
void kvdb::nodemessage(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  // put msg to db
  chat::NodeMessage msg;
  msg.ParseFromArray(sp->data(), sp->data_size());
  putMsg(msg.tonodeid(), msg.incrementid(), msg.SerializeAsString());
  auto key = msgKey(msg.tonodeid(), msg.incrementid());
  call_mmap(sp->session_id(), key);
}

void kvdb::mediaRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  chat::MediaRes mediaRes;
  auto key = std::to_string(mediaRes.nth());
  call_erase_map(sp->session_id(), key);
}

void kvdb::media(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  chat::Media media;
  media.ParseFromArray(sp->data(), sp->data_size());
  std::string value;
  auto status = get(std::to_string(sp->session_id()), value);
  value.append(sp->data(), sp->data_size());
  put(std::to_string(sp->session_id()), value);
  if (value.length() == 
      static_cast<unsigned long>(media.length())) {
    // media intact
    
    unsigned char sha1_result[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, value.data(), value.size());
    SHA1_Final(sha1_result, &ctx);
    std::string sha1(reinterpret_cast<char *>(sha1_result), 
        SHA_DIGEST_LENGTH);
    if (sha1 == media.sha1()) {
      leveldb::WriteBatch batch;
      batch.Delete(std::to_string(sp->session_id()));
      batch.Put(sha1, value);
      db_->Write(leveldb::WriteOptions(), &batch);
      callerase_media_map(std::to_string(sp->session_id()), 
          media.sha1());
    }else {
      db_->Delete(leveldb::WriteOptions(), 
          std::to_string(sp->session_id()));
      throw std::system_error(std::error_code(50032, 
            std::generic_category()),
         "media sha1 check failure");
    }
  }
}

void kvdb::mediaisexistRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  chat::MediaIsExistRes res;
  res.ParseFromArray(sp->data(), sp->data_size());
  if (res.isexist()) {
    call_erase_map(sp->session_id(), "1");
  }else {
    call_erase_map(sp->session_id(), "0");
  }
}

void kvdb::mediacheckRes(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  chat::MediaCheckRes res;
  res.ParseFromArray(sp->data(), sp->data_size());
  if (res.isintact()){
    call_erase_map(sp->session_id(), "1");
  }else {
    call_erase_map(sp->session_id(), "0");
  }
}
void kvdb::msgunreadnoti(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  chat::NodeMessageNoti noti;
  noti.ParseFromArray(sp->data(), sp->data_size());
  queryonemsg(noti.tonodeid(), noti.unreadincrement(),
      [this](const std::string & key, bool * isStop){
        call_map(Session_ID::accept_unread_msg, key);
        *isStop = true;
      });
}

void kvdb::usernoti(Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  if (sp->datatype() == ChatType::loginnoti) {
    put(loginNotiKey(), 
        leveldb::Slice(sp->data(), sp->data_size()));
    call_map(Session_ID::user_noti, loginNotiKey());
  }else if( sp->datatype() == ChatType::addfriendnoti) {
    put(addFriendNotiKey(), 
        leveldb::Slice(sp->data(), sp->data_size()));
    call_map(Session_ID::user_noti, addFriendNotiKey());
  }else if( sp->datatype() == ChatType::addfriendauthorizenoti) {
    put(addFriendAuthorizeNotiKey(), 
        leveldb::Slice(sp->data(), sp->data_size()));
    call_map(Session_ID::user_noti, addFriendAuthorizeNotiKey());
  }else{
    YILOG_ERROR ("usernoti func not accept this type");
  }
}

