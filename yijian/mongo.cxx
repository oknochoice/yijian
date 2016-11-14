#include "mongo.h"
#include "macro.h"

void userNodocNoarr(chat::User & user, bsoncxx::document::view & view) {

    user.set_id(view["_id"].get_oid().value.to_string());
    user.set_realname(view["realname"].
        get_utf8().value.to_string());
    user.set_nickname(view["nickname"].
        get_utf8().value.to_string());
    user.set_icon(view["icon"].
        get_utf8().value.to_string());
    user.set_description(view["description"].
        get_utf8().value.to_string());
    user.set_ismale(view["isMale"].get_bool().value);
    auto code_phone = view["code_phone"].
      get_utf8().value.to_string();
    auto under_line_pos = code_phone.find('_');
    user.set_phoneno(code_phone.substr(0, under_line_pos));
    user.set_countrycode(code_phone.substr(under_line_pos + 1));
    user.set_password(view["password"].
        get_utf8().value.to_string());
    user.set_birthday(view["birthday"].get_int32().value);
    user.set_version(view["version"].get_int32().value);

}

void userinfoDocument(chat::UserInfo & userinfo,
    bsoncxx::document::view & userinfo_view) {

  userinfo.set_tonodeid(userinfo_view["toNodeID"].
      get_utf8().value.to_string());
  userinfo.set_nickname(userinfo_view["nickname"].
      get_utf8().value.to_string());
  userinfo.set_userid(userinfo_view["userID"].
          get_utf8().value.to_string());

}

void deviceDocument(chat::Device & device,
    bsoncxx::document::view & device_view) {

  device.set_os(static_cast<chat::Device::OperatingSystem>
      (device_view["OS"].get_int32().value));
  device.set_osversion(device_view["OSVersion"].get_utf8().value.to_string());
  device.set_clientversion(device_view["clientVersion"].
      get_utf8().value.to_string());
  device.set_appversion(device_view["appVersion"].
      get_utf8().value.to_string());
  device.set_devicemodel(device_view["deviceModel"].
      get_utf8().value.to_string());
  device.set_devicenickname(device_view["deviceNickname"].
      get_utf8().value.to_string());
  device.set_uuid(device_view["UUID"].get_utf8().value.to_string());
  device.set_islogin(device_view["isLogin"].get_bool().value);
  device.set_isconnected(device_view["isConnected"].get_bool().value);
  device.set_isrecivenoti(device_view["isReciveNoti"].get_bool().value);

}

std::shared_ptr<chat::User> userDocoument(bsoncxx::document::view & user_view) {
  YILOG_TRACE ("func: {}. ", __func__);

    auto user_sp = std::make_shared<chat::User>();

    userNodocNoarr(*user_sp, user_view);

    auto friends = user_view["friends"].get_array().value;
    for (auto it = friends.begin();
        it != friends.end();
        ++it) {
      auto doc_view = it->get_document().view();
      auto userinfo = user_sp->mutable_friends()->Add();
      userinfoDocument(*userinfo, doc_view);
    }

    auto blacklist = 
      user_view["blacklist"].get_array().value;
    for (auto it = blacklist.begin();
        it != blacklist.end();
        ++it) {
      auto blackname = user_sp->mutable_blacklist()->Add();
      *blackname = it->get_utf8().value.to_string();
    }

    auto groupids = 
      user_view["groupIDs"].get_array().value;
    for (auto it = groupids.begin();
        it != groupids.end();
        ++it) {
      auto doc_id = it->get_utf8().value.to_string();
      auto groupid = user_sp->mutable_groupids()->Add();
      *groupid = doc_id;
    }

    auto devices = 
      user_view["devices"].get_array().value;
    for (auto it = devices.begin();
        it != devices.end();
        ++it) {
      auto doc_view = it->get_document().view();
      auto device = user_sp->mutable_devices()->Add();
      deviceDocument(*device, doc_view);
    }
    return user_sp;
}

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::array;

mongo_client::mongo_client() {
  mongocxx::uri uri("mongodb://localhost:27017");
  client_ = mongocxx::client(uri);
}


mongo_client::~mongo_client () {
}

// user
void mongo_client::insertUser(const chat::Register & enroll) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  auto builder = document{};
  bsoncxx::document::value doc_value = builder
    << "realname" << ""
    << "nickname" << enroll.nickname()
    << "icon" << ""
    << "description" << ""
    << "isMale" << ""
    << "code_phone" << enroll.countrycode() + "_" + enroll.phoneno()
    << "password" << enroll.password()
    << "birthday" << 0
    << "version" << "1"
    << "friends" << open_array << close_array
    << "blacklist" << open_array << close_array
    << "groupIDs" << open_array << close_array
    << "devices" << open_array << close_array
    << finalize;
  user_collection.insert_one(doc_value.view());
}

void mongo_client::updateDevice(
    const std::string & userID, const chat::Device & device) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  user_collection.update_one(
      document{} << "_id" << bsoncxx::oid(userID)
      << "devices.UUID" << device.uuid() 
      << finalize,
      document{} << "$set" << open_document 
      << "devices.$" << open_document
      << "OS" << device.os()
      << "OSVersion" << device.osversion()
      << "clientVersion" << device.clientversion()
      << "appVersion" << device.appversion()
      << "deviceModel" << device.devicemodel()
      << "UUID" << device.uuid()
      << "isLogin" << device.islogin()
      << "isConnected" << device.isconnected()
      << "isReciveNoti" << device.isrecivenoti() << close_document
      << close_document 
      << finalize
      );
}
void mongo_client::insertDevice(
    const std::string & userID, const chat::Device & device) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  user_collection.update_one(
      document{} << "_id" << bsoncxx::oid(userID)
      << finalize,
      document{} << "$push" << open_document 
      << "devices" << open_document
      << "OS" << device.os()
      << "OSVersion" << device.osversion()
      << "clientVersion" << device.clientversion()
      << "appVersion" << device.appversion()
      << "deviceModel" << device.devicemodel()
      << "UUID" << device.uuid()
      << "isLogin" << device.islogin()
      << "isConnected" << device.isconnected()
      << "isReciveNoti" << device.isrecivenoti() << close_document
      << close_document 
      << finalize
      );
}


std::shared_ptr<chat::User> 
mongo_client::queryUser(const std::string & phoneNo, 
    const std::string & countryCode) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  auto maybe_result = user_collection.find_one(
      document{} << "code_phone" << countryCode + "_" + phoneNo
      << finalize);
  if (maybe_result) {
    auto user_view = maybe_result->view();
    return userDocoument(user_view);
  }else {
    throw std::system_error(std::error_code(40000, std::generic_category()),
        "no this user");
  }
}

std::shared_ptr<chat::User> 
mongo_client::queryUser(const std::string & userID) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  auto maybe_result = user_collection.find_one(
      document{} << "_id" << bsoncxx::oid(userID)
      << finalize);
  if (maybe_result) {
    auto user_view = maybe_result->view();
    return userDocoument(user_view);
  }else {
    throw std::system_error(std::error_code(40001, std::generic_category()),
        "no this user");
  }
}

std::shared_ptr<chat::AddFriendAuthorizeInfo>
mongo_client::addFriendAuthorize(const std::string & inviter, 
      const std::string & inviterNickname,
      const std::string & invitee,
      const std::string & inviteeNickname) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  // check old friendship
  auto old_inviter_friend = user_collection.find_one(
      document{} << "_id" << bsoncxx::oid(inviter)
      << "friends.userID" << invitee 
      << finalize);
  auto old_invitee_friend = user_collection.find_one(
      document{} << "_id" << bsoncxx::oid(invitee)
      << "friends.userID" << inviter 
      << finalize);
  if (unlikely(old_inviter_friend && old_invitee_friend)) {
    throw std::system_error(std::error_code(40010, std::generic_category()),
        "already friend");
  }
  if (unlikely(old_inviter_friend)) {
    auto inviter_check = user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(inviter)
        << "friends.userID" << invitee 
        << finalize,
        document{} << "$set" << open_document
        << "friends.$.userID" << "-1" << close_document
        << finalize);
    YILOG_ERROR("add friend redundancy userid {}", inviter);
  }
  if (unlikely(old_invitee_friend)) {
    auto invitee_check = user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(invitee)
        << "friends.userID" << inviter 
        << finalize,
        document{} << "$set" << open_document
        << "friends.$.userID" << "-1" << close_document
        << finalize);
    YILOG_ERROR("add friend redundancy userid {}", invitee);
  }

  // create new node
  auto messageNode_collection = db["messageNode"];
  auto maybe_result = messageNode_collection.insert_one(
      document{} << "authorize" << chat::MessageNode::peer
      << "creatorID" << inviter
      << "nickname" << ""
      << "managerIDs" << open_array << close_array
      << "memebers" << open_array 
      << inviter << invitee << close_array
      << finalize);
  auto obj_id = maybe_result->inserted_id().get_oid().value.to_string();
  if (likely(maybe_result)) {
    auto inviter_result = user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(inviter)
        << finalize,
        document{} << "$addToSet" << open_document 
        << "friends" << open_document
        << "toNodeID" << obj_id
        << "nickname" << inviteeNickname
        << "userID" << invitee << close_document
        << close_document << finalize);
    auto inviter_version = user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(inviter)
        << finalize,
        document{} << "$inc" << "version"
        << finalize);
    auto invitee_result = user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(invitee)
        << finalize,
        document{} << "$addToSet" << open_document 
        << "friends" << open_document
        << "toNodeID" << obj_id
        << "nickname" << inviterNickname
        << "userID" << inviter << close_document
        << close_document << finalize);
    auto invitee_version = user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(invitee)
        << finalize,
        document{} << "$inc" << "version"
        << finalize);
    if (unlikely(!inviter_result)) {
      YILOG_ERROR("mongo add friend failure, messageNode {} user id {}"
          , obj_id, inviter);
      throw std::system_error(std::error_code(40004, std::generic_category()),
          "create friend user info failure");
    }else if(unlikely(!invitee_result)) {
      YILOG_ERROR("mongo add friend failure, messageNode {} user id {}"
          , obj_id, invitee);
      throw std::system_error(std::error_code(40005, std::generic_category()),
          "create friend user info failure");
    }else if(unlikely(!inviter_version)) {
      YILOG_ERROR("mongo update user version failure, user id {}"
          , inviter);
      throw std::system_error(std::error_code(40006, std::generic_category()),
          "create friend user info failure");
    }else if(unlikely(!invitee_version)) {
      YILOG_ERROR("mongo update user version failure, user id {}"
          , invitee);
      throw std::system_error(std::error_code(40007, std::generic_category()),
          "create friend user info failure");
    }
    auto info = std::make_shared<chat::AddFriendAuthorizeInfo>();
    info->set_tonodeid(obj_id);
    auto inviter_v = user_collection.find_one(
        document{} << "_id" << bsoncxx::oid(inviter)
        << finalize
        );
    info->set_inviteruserversion(inviter_v->view()["version"].get_int32());
    auto invitee_v = user_collection.find_one(
        document{} << "_id" << bsoncxx::oid(invitee)
        << finalize
        );
    info->set_inviteeuserversion(invitee_v->view()["version"].get_int32());
    return info;
  }else {
    throw std::system_error(std::error_code(40008, std::generic_category()),
        "create message node for friend error");
  }
}


std::shared_ptr<chat::CreateGroupRes>
mongo_client::createGroup(const chat::CreateGroup & cGroup) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto messageNode_collection = db["messageNode"];
  auto user_collection = db["user"];
  auto memebersBuild = array{};
  for (auto & memberid: cGroup.membersid()) {
    memebersBuild << memberid;
  }
  auto memebers = memebersBuild << finalize;
  auto maybe_result = messageNode_collection.insert_one(
      document{} << "authorize" << chat::MessageNode::peer
      << "creatorID" << cGroup.userid()
      << "nickname" << cGroup.nickname() 
      << "managerIDs" << open_array << close_array
      << "memebers" << memebers 
      << finalize);
  if (likely(maybe_result)) {
    auto obj_id = maybe_result->inserted_id().get_oid().value.to_string();
    for (auto & memberid: cGroup.membersid()) {
      auto updateOne_result = user_collection.update_one(
          document{} << "_id" << bsoncxx::oid(memberid)
          << finalize,
          document{} << "$addToSet" << open_document
          << "groupIDs" << obj_id
          << close_document
          << finalize);
    }
    auto re = std::make_shared<chat::CreateGroupRes>();
    re->set_tonodeid(obj_id);
    re->set_nickname(cGroup.nickname());
    return re;
  }else {
    throw std::system_error(std::error_code(40009, std::generic_category()),
        "create message node for group error");
  }
  
}

std::shared_ptr<chat::GroupAddMemberRes>
mongo_client::addMembers2Group(chat::GroupAddMember & groupMember) {

  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto messageNode_collection = db["messageNode"];
  auto user_collection = db["user"];

  for (auto & memberid: groupMember.membersid()) {
    auto updateOne_result = user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(memberid)
        << finalize,
        document{} << "$addToSet" << open_document
        << "groupIDs" << groupMember.groupid()
        << close_document
        << finalize);
  }
  auto addGroupRes = std::make_shared<chat::GroupAddMemberRes>();
  addGroupRes->set_groupid(groupMember.groupid());
  return addGroupRes;
}

namespace yijian {

  namespace threadCurrent {
    mongo_client * mongoClient() {
      static thread_local auto client = new mongo_client();
      return client;
    }
    inmem_client * inmemClient() {
      static thread_local auto client = new inmem_client(SERVER_NAME);
      return client;
    }
  }
}

