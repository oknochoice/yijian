#include "mongo.h"
#include <chrono>

void userNodocNoarr(chat::User & user, bsoncxx::document::view & view) {

  YILOG_TRACE ("func: {}. ", __func__);
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
    user.set_countrycode(view["countryCode"].get_utf8().value.to_string());
    user.set_phoneno(view["phoneNo"].get_utf8().value.to_string());
    user.set_password(view["password"].
        get_utf8().value.to_string());
    user.set_birthday(view["birthday"].get_int32().value);
    user.set_version(view["version"].get_int32().value);

}

void userinfoDocument(chat::UserInfo & userinfo,
    bsoncxx::document::view & userinfo_view) {

  YILOG_TRACE ("func: {}. ", __func__);
  userinfo.set_tonodeid(userinfo_view["toNodeID"].
      get_utf8().value.to_string());
  userinfo.set_userid(userinfo_view["userID"].
          get_utf8().value.to_string());

}

void deviceDocument(chat::Device & device,
    bsoncxx::document::view & device_view) {

  YILOG_TRACE ("func: {}. ", __func__);
  device.set_os(static_cast<chat::Device::OperatingSystem>
      (device_view["OS"].get_int32().value));
  device.set_devicemodel(device_view["deviceModel"].
      get_utf8().value.to_string());
  auto it = device_view["deviceNickname"];
  if (it) {
    device.set_devicenickname(it.get_utf8().value.to_string());
  }else {
    device.set_devicenickname("");
  }
  device.set_uuid(device_view["UUID"].get_utf8().value.to_string());

}

void messagenodeDocument(chat::MessageNode & messagenode,
    bsoncxx::document::view & messagenode_view) {
  YILOG_TRACE ("func: {}. ", __func__);
  messagenode.set_id(messagenode_view["_id"].get_oid().value.to_string());
  messagenode.set_authorize(
      static_cast<chat::MessageNode::Authorize>(
      messagenode_view["authorize"].get_int32().value));
  messagenode.set_creatorid(messagenode_view["creatorID"].
      get_utf8().value.to_string());
  messagenode.set_nickname(messagenode_view["nickname"].
      get_utf8().value.to_string());
  messagenode.set_version(messagenode_view["version"].get_int32().value);
  auto managerIDs = messagenode_view["managerIDs"].get_array().value;
  for (auto it = managerIDs.begin();
      it != managerIDs.end();
      ++it) {
    messagenode.add_managerids(it->get_utf8().value.to_string());
  }
  auto members = messagenode_view["members"].get_array().value;
  for (auto it = members.begin();
      it != members.end();
      ++it) {
    messagenode.add_members(it->get_utf8().value.to_string());
  }
}

void nodemessageDocument(chat::NodeMessage & nodemessage,
    bsoncxx::document::view & nodemessage_view) {
  YILOG_TRACE ("func: {}. ", __func__);
  nodemessage.set_id(nodemessage_view["_id"].get_oid().value.to_string());
  nodemessage.set_fromuserid(nodemessage_view["fromUserID"].
      get_utf8().value.to_string());
  nodemessage.set_tonodeid(nodemessage_view["toNodeID"].
      get_utf8().value.to_string());
  nodemessage.set_incrementid(nodemessage_view["incrementID"].
      get_int32().value);
  nodemessage.set_type(static_cast<chat::MediaType>(
        nodemessage_view["type"].get_int32().value));
  nodemessage.set_content(nodemessage_view["content"].
      get_utf8().value.to_string());
}

/*
void friendreqDocument(chat::FriendRequest & frdReq, 
    bsoncxx::document::view & view) {
  frdReq.set_isinviter(view["isInviter"].get_bool().value);
  frdReq.set_userid(view["userID"].get_utf8().value.to_string());
}
*/

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

    auto groupnodeids = 
      user_view["groupNodeIDs"].get_array().value;
    for (auto it = groupnodeids.begin();
        it != groupnodeids.end();
        ++it) {
      auto doc_id = it->get_utf8().value.to_string();
      auto groupid = user_sp->mutable_groupnodeids()->Add();
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


mongo_client::mongo_client(std::string serverName)
  : serverName_(serverName){
  YILOG_TRACE ("func: {}. ", __func__);
  mongocxx::uri uri("mongodb://localhost:27017");
  client_ = mongocxx::client(uri);
  journal_concern_ = mongocxx::write_concern();
  journal_concern_.journal(true);
  journal_insert_ = mongocxx::options::insert();
  journal_insert_.write_concern(journal_concern_);
  journal_update_ = mongocxx::options::update();
  journal_update_.write_concern(journal_concern_);
}


mongo_client::~mongo_client () {
  YILOG_TRACE ("func: {}. ", __func__);
}

// user
std::string& mongo_client::insertUser(const chat::Register & enroll) {
  YILOG_TRACE ("func: {}. ", __func__);

  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  auto builder = document{};
  bsoncxx::document::value doc_value = builder
    << "realname" << ""
    << "nickname" << enroll.nickname()
    << "icon" << ""
    << "description" << ""
    << "isMale" << true
    << "countryCode" << enroll.countrycode() 
    << "phoneNo" << enroll.phoneno()
    << "password" << enroll.password()
    << "birthday" << 0
    << "version" << 1
    << "friends" << open_array << close_array
    << "blacklist" << open_array << close_array
    << "groupNodeIDs" << open_array << close_array
    << "devices" << open_array << close_array
    << finalize;
  try {
    auto maybe_result = user_collection.insert_one(doc_value.view(),
        journal_insert_);
    static thread_local std::string id;
    id.clear();
    id = maybe_result->inserted_id().get_oid().value.to_string();
    YILOG_TRACE ("func: {}. id {}", __func__, id);
    return id;
  }catch(const std::system_error & e){
    YILOG_ERROR ("insert user failure, countrycode:{}, phone: {}."
        "system_error code:{}, what:{}.", 
        enroll.countrycode(), enroll.phoneno(), e.code().value(), e.what());
    throw std::system_error(std::error_code(40011, std::generic_category()),
        "insert user failure");
  }

}

void mongo_client::updateDevice(
    const std::string & userID, const chat::Device & device) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  try {
    user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(userID)
        << "devices.UUID" << device.uuid() 
        << finalize,
        document{} << "$set" << open_document 
        << "devices.$" << open_document
        << "OS" << device.os()
        << "deviceModel" << device.devicemodel()
        << "deviceNickname" << device.devicenickname()
        << "UUID" << device.uuid()
        << close_document << close_document
        << finalize,
        journal_update_
        );
  }catch(const std::system_error & e){
    YILOG_ERROR ("update user device failure, uuid :{} ."
        "system_error code:{}, what:{}.", 
        device.uuid(), e.code().value(), e.what());
    throw std::system_error(std::error_code(40000, std::generic_category()),
        "update user device failure");
  }
}
void mongo_client::insertDevice(
    const std::string & userID, const chat::Device & device) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  try {
    user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(userID)
        << finalize,
        document{} << "$push" << open_document 
        << "devices" << open_document
        << "OS" << device.os()
        << "deviceModel" << device.devicemodel()
        << "UUID" << device.uuid()
        << close_document << close_document
        << finalize,
        journal_update_
        );
  }catch(const std::system_error & e){
    YILOG_ERROR ("insert user device failure, uuid :{} ."
        "system_error code:{}, what:{}.", 
        device.uuid(), e.code().value(), e.what());
    throw std::system_error(std::error_code(40001, std::generic_category()),
        "insert user device failure");
  }
}


std::shared_ptr<chat::User> 
mongo_client::queryUser(const std::string & phoneNo, 
    const std::string & countryCode) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  auto maybe_result = user_collection.find_one(
      document{} << "countryCode" << countryCode 
      << "phoneNo" << phoneNo
      << finalize);
  if (unlikely(!maybe_result)) {
    YILOG_ERROR ("query user failure, countrycode:{}, phone:{} .", 
        countryCode, phoneNo);
    throw std::system_error(std::error_code(40002, std::generic_category()),
        "no this user");
  }
  auto user_view = maybe_result->view();
  return userDocoument(user_view);
}

std::shared_ptr<chat::User> 
mongo_client::queryUser(const std::string & userID) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  auto maybe_result = user_collection.find_one(
      document{} << "_id" << bsoncxx::oid(userID)
      << finalize);
  if (unlikely(!maybe_result)) {
    YILOG_ERROR ("query user failure, userid: {} .", 
        userID);
    throw std::system_error(std::error_code(40003, std::generic_category()),
        "no this user");
  }
  auto user_view = maybe_result->view();
  return userDocoument(user_view);
}

std::shared_ptr<chat::AddFriendRes>
mongo_client::addFriend(const chat::AddFriend & addfrd) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto db = client_["chatdb"];
  auto addfriend_col = db["addFriend"];
  
  std::string littleIDBigID;
  if (addfrd.inviterid() < addfrd.inviteeid()) {
    littleIDBigID = addfrd.inviterid() + "_" + addfrd.inviteeid();
  }else {
    littleIDBigID = addfrd.inviteeid() + "_" + addfrd.inviterid();
  }
  try {
    // track
    auto now = std::chrono::high_resolution_clock::now();
    auto ts = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    addfriend_col.insert_one(
        document{}
        << "littleIDBigID" << littleIDBigID
        << "inviter" << addfrd.inviterid()
        << "invitee" << addfrd.inviteeid()
        << "toNodeID" << ""
        << "status" << "request"
        << "timestamp" << ts.count() 
        << finalize,
        journal_insert_);
    // create new node
    auto messageNode_collection = db["messageNode"];
    auto msgNode_result = messageNode_collection.insert_one(
        document{} << "authorize" << chat::MessageNode::peer
        << "creatorID" << addfrd.inviterid()
        << "nickname" << ""
        << "managerIDs" << open_array << close_array
        << "memebers" << open_array 
        << addfrd.inviterid() << addfrd.inviteeid() << close_array
        << finalize,
        journal_insert_);
    auto msgNodeid = 
        msgNode_result->inserted_id().get_oid().value.to_string();
    // track
    addfriend_col.update_one(
        document{}
        << "littleIDBigID" << littleIDBigID
        << finalize,
        document{} << "$set" << open_document
        << "toNodeID" << msgNodeid
        << "status" << "createNode" << close_document
        << finalize,
        journal_update_);
    // create messge node count
    auto messageNode_count_col = db["messageNodeCount"];
    auto count_result = messageNode_count_col.insert_one(
        document{} << "nodeID" << msgNodeid
        << "count" << 0
        << finalize,
        journal_insert_);
    // track
    addfriend_col.update_one(
        document{}
        << "littleIDBigID" << littleIDBigID
        << finalize,
        document{} << "$set" << open_document
        << "status" << "createNodeCount" << close_document
        << finalize,
        journal_update_);
    // insert request message
    auto requestmsg = chat::NodeMessage();
    requestmsg.set_fromuserid(addfrd.inviterid());
    requestmsg.set_tonodeid(msgNodeid);
    requestmsg.set_touserid_outer(addfrd.inviteeid());
    requestmsg.set_type(chat::MediaType::TEXT);
    requestmsg.set_content(addfrd.msg());
    
    insertMessage(requestmsg);

    // return message
    auto res = std::make_shared<chat::AddFriendRes>();
    res->set_tonodeid(msgNodeid);
    res->set_inviterid(addfrd.inviterid());
    res->set_inviteeid(addfrd.inviteeid());
    
    return res;
  }catch (std::system_error & e ) {
    YILOG_ERROR ("add friend failure, inviter:{}, invitee:{} \n"
        "system_error code:{}, what:{}.", 
        addfrd.inviterid(), addfrd.inviteeid(), e.code().value(), e.what());
    throw std::system_error(std::error_code(40004, std::generic_category()),
        "add friend failure");
  }

}

void mongo_client::addFriendAuthorize(const std::string & inviter, 
      const std::string & invitee) {
  YILOG_TRACE ("func: {}. ", __func__);

  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  auto addfriend_col = db["addFriend"];
  std::string littleIDBigID;
  if (inviter< invitee) {
    littleIDBigID = inviter + "_" + invitee;
  }else {
    littleIDBigID = invitee + "_" + inviter;
  }
  // find tonodeid
  auto maybe_result = addfriend_col.find_one(
      document{} << "littleIDBigID" << littleIDBigID
      << finalize);
  if (!maybe_result) {
    YILOG_ERROR ("not find add friend request, inviter: {}, invitee {}",
        inviter, invitee);
    throw std::system_error(std::error_code(40008, std::generic_category()),
        "not find add friend request");
  }
  auto status = maybe_result->view()["status"].
    get_utf8().value.to_string();
  if (unlikely(status != "createNodeCount")) {
    YILOG_ERROR ("add friend authorize status: {}, inviter: {}, invitee {}",
        status, inviter, invitee);
    throw std::system_error(std::error_code(40009, std::generic_category()),
        "add friend authorize info not consistently");
  }
  auto tonodeid = maybe_result->view()["toNodeID"].
    get_utf8().value.to_string();
  try {
    // track
    addfriend_col.update_one(
        document{}
        << "littleIDBigID" << littleIDBigID
        << finalize,
        document{} << "$set" << open_document
        << "status" << "agree" << close_document
        << finalize,
        journal_update_);
    // inviter add
    user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(inviter)
        << finalize,
        document{} << "$addToSet" << open_document
        << "friends" << open_document
        << "toNodeID" << tonodeid
        << "userID" << invitee
        << close_document << close_document
        << "$inc" << open_document 
        << "version" << 1 << close_document
        << finalize,
        journal_update_);
    // track
    addfriend_col.update_one(
        document{}
        << "littleIDBigID" << littleIDBigID
        << finalize,
        document{} << "$set" << open_document
        << "status" << "inviter" << close_document
        << finalize,
        journal_update_);
    // insert unread node
    insertUnreadNode(inviter, tonodeid);
    // track 
    addfriend_col.update_one(
        document{}
        << "littleIDBigID" << littleIDBigID
        << finalize,
        document{} << "$set" << open_document
        << "status" << "userUnreadInviter" << close_document
        << finalize,
        journal_update_);
    // invitee add
    user_collection.update_one(
        document{} << "_id" << bsoncxx::oid(invitee)
        << finalize,
        document{} << "$addToSet" << open_document
        << "friends" << open_document
        << "toNodeID" << tonodeid
        << "userID" << inviter
        << close_document << close_document
        << "$inc" << open_document 
        << "version" << 1 << close_document
        << finalize,
        journal_update_);
    // track
    addfriend_col.update_one(
        document{}
        << "littleIDBigID" << littleIDBigID
        << finalize,
        document{} << "$set" << open_document
        << "status" << "invitee" << close_document
        << finalize,
        journal_update_);
    // insert unread node
    insertUnreadNode(invitee, tonodeid);
    // track 
    addfriend_col.update_one(
        document{}
        << "littleIDBigID" << littleIDBigID
        << finalize,
        document{} << "$set" << open_document
        << "status" << "finish" << close_document
        << finalize,
        journal_update_);
  }catch(std::system_error & e) {
    YILOG_ERROR ("add friend authorize failre in (id insert to friends).\n"
        " inviter:{}, invitee{} \n"
        "system_error code:{}, what:{}.", 
        inviter, invitee, e.code().value(), e.what());
    throw std::system_error(std::error_code(40005, std::generic_category()),
        "add friend authorize failre");
  }

}
void mongo_client::queryAddfriendInfo(
      std::function<void(std::shared_ptr<chat::QueryAddfriendInfoRes>)> && func,
      const std::string & userid,
      const int limit) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto addfriend_col = db["addFriend"];
  try {
    auto find = mongocxx::options::find{};
    auto hint = mongocxx::hint(
        document{} 
        << "timestamp" << 1
        << finalize);
    find.sort(document{} << "timestamp" << -1 << finalize);
    find.limit(limit);
    find.hint(hint);
    /*
    std::string regexPre;
    regexPre = "/""^" + userid + "/";
    std::string regexEnd;
    regexEnd = "/" + userid + "$""/";
    */
    auto cursor = addfriend_col.find(
        document{} << "$and" << open_array << open_document
        << "$or" << open_array
        << open_document << "inviter" << userid << close_document
        << open_document << "invitee" << userid << close_document
        << close_array << close_document << open_document
        << "status" << open_document 
        << "$not"  << open_document 
        << "$eq" << "finish" << close_document
        << close_document << close_document << close_array
        << finalize);
    for (auto doc: cursor) {
      auto re = std::make_shared<chat::QueryAddfriendInfoRes>();
      re->set_inviter(doc["inviter"].get_utf8().value.to_string());
      re->set_invitee(doc["invitee"].get_utf8().value.to_string());
      re->set_tonodeid(doc["toNodeID"].get_utf8().value.to_string());
      func(re);
    }
  }catch(std::system_error & e) {
    YILOG_ERROR ("query addfriend authorize info failre.\n"
        "system_error code:{}, what:{}.", 
        e.code().value(), e.what());
    throw std::system_error(std::error_code(40010, std::generic_category()),
        "query addfriend authorize info failre");
  }
}

// group
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
  try {
    // create message node
    auto result = messageNode_collection.insert_one(
        document{} << "authorize" << chat::MessageNode::peer
        << "creatorID" << cGroup.userid()
        << "nickname" << cGroup.nickname() 
        << "version" << 1
        << "managerIDs" << open_array << close_array
        << "memebers" << memebers 
        << finalize,
        journal_insert_);

    auto obj_id = result->inserted_id().get_oid().value.to_string();
    // add message node count
    auto messageNode_count_col = db["messageNodeCount"];
    messageNode_count_col.insert_one(
        document{} << "nodeID" << obj_id
        << "count" << 0
        << finalize,
        journal_insert_);

    // insert user unread
    std::vector<mongocxx::model::write> writes;
    for (auto & memberid: cGroup.membersid()) {
      auto doc =  document{} << "insertOne" << open_document
          << "userID" << memberid
          << "toNodeID" << obj_id
          << "unreadIncrement" << 1
          << "readedIncrement" << 1
          << close_document << finalize;
      auto write = mongocxx::model::write(
          mongocxx::model::insert_one(doc.view()));
      writes.push_back(std::move(write));
    }
    auto userUnread = db["userUnread"];
    mongocxx::options::bulk_write write_c;
    write_c.ordered(false);
    write_c.write_concern(journal_concern_);
#warning may be need check who insered failure
    userUnread.bulk_write(writes, write_c);
    // return
    auto re = std::make_shared<chat::CreateGroupRes>();
    re->set_tonodeid(obj_id);
    re->set_nickname(cGroup.nickname());
    return re;
  }catch(std::system_error & e) {
    YILOG_ERROR("create message node failure, creater id\n"
        "system_error code:{}, what:{}.", 
        cGroup.userid(), e.code().value(), e.what());
    throw std::system_error(std::error_code(40021, std::generic_category()),
        "create message node for group error");
  }
  
}

std::shared_ptr<chat::GroupAddMemberRes>
mongo_client::addMembers2Group(chat::GroupAddMember & groupMember) {

  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto messageNode_collection = db["messageNode"];
  auto user_collection = db["user"];

  // message node add member
  auto arraybuilder = array{};
  for (auto & memberid: groupMember.membersid()) {
    arraybuilder << memberid;
  }
  auto membersid = arraybuilder << finalize;
  try {
    messageNode_collection.update_one(
        document{} << "_id" << bsoncxx::oid(groupMember.tonodeid())
        << finalize,
        document{} << "$addToSet" << open_document
        << "membersid" << open_document
        << "$each" << membersid << close_document << close_document
        << "$inc" << open_document 
        << "version" << 1 << close_document
        << finalize,
        journal_update_);

    // insert user unread
    std::vector<mongocxx::model::write> writes;
    for (auto & memberid: groupMember.membersid()) {
      auto doc =  document{} << "insertOne" << open_document
          << "userID" << memberid
          << "toNodeID" << memberid
          << "unreadIncrement" << 1
          << "readedIncrement" << 1
          << close_document << finalize;
      auto write = mongocxx::model::write(
          mongocxx::model::insert_one(doc.view()));
      writes.push_back(std::move(write));
    }
    auto userUnread = db["userUnread"];
    mongocxx::options::bulk_write write_c;
    write_c.ordered(false);
    write_c.write_concern(journal_concern_);
#warning may be need check who insered failure
    userUnread.bulk_write(writes, write_c);
    // return
    auto addGroupRes = std::make_shared<chat::GroupAddMemberRes>();
    addGroupRes->set_tonodeid(groupMember.tonodeid());
    return addGroupRes;
  }catch(std::system_error & e) {
    YILOG_ERROR("create message node failure, creater id\n"
        "system_error code:{}, what:{}.", 
        groupMember.tonodeid(), e.code().value(), e.what());
    throw std::system_error(std::error_code(40022, std::generic_category()),
        "add group member failure");
  }
}


std::shared_ptr<chat::MessageNode>
mongo_client::queryNode(const std::string & nodeID) {

  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto messageNode_collection = db["messageNode"];

  auto messagenode_result = messageNode_collection.find_one(
      document{} << "_id" << bsoncxx::oid(nodeID)
      << finalize);
  if (unlikely(!messagenode_result)) {
    YILOG_ERROR ("query message node failure, nodeid: {}", nodeID);
    throw std::system_error(std::error_code(40023, std::generic_category()),
        "query message node failure");
  }
  auto messagenode_view = messagenode_result->view();
  auto node_sp = std::make_shared<chat::MessageNode>();
  messagenodeDocument(*node_sp, messagenode_view);
  return node_sp;
}

// message 
std::shared_ptr<chat::NodeMessageRes> 
mongo_client::insertMessage(chat::NodeMessage & message) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto nodemessage_collection = db["nodeMessage"];
  auto messageNode_count_col = db["messageNodeCount"];
  try {
    auto node_count = messageNode_count_col.find_one_and_update(
        document{} << "nodeID" << message.tonodeid()
        << finalize,
        document{} << "$inc" << open_document
        << "count" << 1 << close_document
        << finalize);
    int32_t incrementid = 0;
    if (node_count) {
      incrementid =  node_count->view()["count"].get_int32().value;
    }else {
      YILOG_ERROR("not find tonodeid {}, in message node count"
          , message.tonodeid());
      throw std::system_error(std::error_code(40050, std::generic_category()),
          "not find tonodeid");
    }

    nodemessage_collection.insert_one(
        document{} << "fromUserID" << message.fromuserid()
        << "toNodeID" << message.tonodeid()
        << "incrementID" << incrementid
        << "type" << message.type()
        << "content" << message.content()
        << finalize);
    auto msgRes = std::make_shared<chat::NodeMessageRes>();
    msgRes->set_tonodeid(message.tonodeid());
    msgRes->set_incrementid(incrementid);
    return msgRes;
  }catch(std::system_error & e) {
    YILOG_ERROR("insert message failure\n"
        "system_error code:{}, what:{}.", 
         e.code().value(), e.what());
    throw std::system_error(std::error_code(40051, std::generic_category()),
        "insert message failure");
  }
}

std::shared_ptr<chat::NodeMessage>
mongo_client::queryMessage(const std::string & tonodeid, 
    const int32_t incrementid) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto nodemessage_collection = db["nodeMessage"];
  auto maybe_result = nodemessage_collection.find_one(
      document{} << "toNodeID" << tonodeid
      << "incrementID" << incrementid
      << finalize);

  if (maybe_result) {
    auto message_sp = std::make_shared<chat::NodeMessage>();
    auto view = maybe_result->view();
    nodemessageDocument(*message_sp, view);
    return message_sp;
  }else {
    YILOG_ERROR("can not find message tonodeid {}, incrementid {}.", 
        tonodeid, incrementid);
    throw std::system_error(std::error_code(40052, std::generic_category()),
        "missing message");
  }
}

void mongo_client::queryMessage(chat::QueryMessage & query, 
      std::function<void(std::shared_ptr<chat::NodeMessage>)> && func) {
  YILOG_TRACE ("func: {}. ", __func__);

  auto db = client_["chatdb"];
  auto nodemessage_collection = db["nodeMessage"];
  auto builder = document{};
  builder << "toNodeID" << query.tonodeid();
  if (query.toincrementid() != 0) {
    builder << "incrementID" << open_document
      << "$gte" << query.fromincrementid() << close_document;
  }else {
    builder << "$and" << open_array << open_document
      << "incrementID" << open_document 
      << "$gte" << query.fromincrementid() 
      << close_document << close_document << open_document
      << "incrementID" << open_document
      << "$lt" << query.toincrementid()
      << close_document << close_document << close_array;
  }
  auto filter = builder << finalize;
  auto opt = mongocxx::options::find{};
  opt.sort(document{} << "incrementID" << -1 << finalize);
  auto cursor = nodemessage_collection.find(filter.view(), opt);
  
  for (auto doc: cursor) {
    auto message_sp = std::make_shared<chat::NodeMessage>();
    nodemessageDocument(*message_sp, doc);
    func(message_sp);
  }
}


// media
void mongo_client::insertMedia(
    const std::vector<chat::Media> & media_vec) {
  YILOG_TRACE ("func: {}. ", __func__);

  SHA_CTX ctx;
  SHA1_Init(&ctx);

  for (auto media: media_vec) {
    auto content = media.content();
    SHA1_Update(&ctx, content.data(), content.size());
  }

  unsigned char sha1_result[SHA_DIGEST_LENGTH];
  SHA1_Final(sha1_result, &ctx);

  std::string sha1(reinterpret_cast<char *>(sha1_result), SHA_DIGEST_LENGTH);

  if (unlikely(sha1 !=
        media_vec.front().sha1())) {
    throw std::system_error(std::error_code(40060, std::generic_category()), "sha1 check failure");
  }

  std::string content;

  for (auto media: media_vec) {
    content.append(media.content());
  }

  auto db = client_["chatdb"];
  auto media_col = db["media"];
  try {
  auto maybe_result = media_col.insert_one(
      document{} << "sha1" << sha1
      << "type" << media_vec.front().type()
      << "content" << content
      << finalize);
  }catch (std::system_error & e) {
    YILOG_ERROR ("media insert failure, sha1: {}"
        "system_error code:{}, what:{}.", 
        sha1, e.code().value(), e.what());
    throw std::system_error(std::error_code(40061, std::generic_category()),
        "media insert failure");
  }
}


void mongo_client::queryMedia(const std::string & sha1, 
    std::vector<std::shared_ptr<chat::Media>> & media_vec,
    int32_t maxLength) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto media_col = db["media"];
  auto maybe_result = media_col.find_one(
      document{} << "sha1" << sha1
      << finalize);
  if (unlikely(!maybe_result)) {
    YILOG_ERROR ("media not find, sha1: {}.",
        sha1);
    throw std::system_error(std::error_code(40062, std::generic_category()),
        "media not find");
  }
  auto doc = maybe_result->view();
  auto type = doc["type"].get_int32().value;
  auto content = doc["content"].get_utf8().value.to_string();

  auto media_sp = std::make_shared<chat::Media>();
  int i = 0;
  std::size_t pos = 0;
  do {
    media_sp->set_sha1(sha1);
    media_sp->set_nth(i);
    media_sp->set_length(content.size());
    media_sp->set_type(static_cast<chat::MediaType>(type));
    auto length = maxLength - media_sp->ByteSize();
    auto subContent = content.substr(pos, length);
    media_sp->set_content(subContent);
    media_vec.push_back(media_sp);
    ++i;
    pos += length;
  }while(pos >= content.size());

}


bool mongo_client::mediaIsExist(const std::string & sha1) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto media_col = db["media"];
  auto maybe_result = media_col.find_one(
      document{} << "sha1" << sha1
      << finalize);
  bool isExist = false;
  if (maybe_result) {
    isExist = true;
  }
  return isExist;
}


void mongo_client::devices(const chat::NodeUser & node_user, 
      std::function<void(chat::ConnectInfoLittle&)> && func) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto connectinfo_col = db["connectInfo"];
  auto cursor = connectinfo_col.find(
      document{} << "userID" << node_user.touserid()
      << "serverName" << serverName_
      << "isLogin" << true
      << finalize);
  auto infolittle = chat::ConnectInfoLittle();
  for (auto doc: cursor) {
    infolittle.set_uuid(doc["UUID"].get_utf8().value.to_string());
    infolittle.set_isconnected(doc["isConnected"].get_bool().value);
    infolittle.set_isrecivenoti(doc["isReciveNoti"].get_bool().value);
    func(infolittle);
  }
  
}

void mongo_client::insertUUID(
    const chat::ConnectInfo & connectInfo) {

  YILOG_TRACE ("func: {}. ", __func__);
  
  auto db = client_["chatdb"];
  auto connectinfo_col = db["connectInfo"];
  // session map {userid: sessionid}
  auto sessionbuilder = array{};
  for (auto & map: connectInfo.users()) {
    sessionbuilder << open_document
      << "userID" << map.first 
      << "sessionID" << map.second << close_document;
  }
  auto users_array = sessionbuilder << finalize;
  // insert
  try {
    auto now = std::chrono::high_resolution_clock::now();
    auto ts = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
  auto maybe_result = connectinfo_col.insert_one(
      document{} << "UUID" << connectInfo.uuid()
      << "userID" << connectInfo.userid()
      << "isLogin" << connectInfo.islogin()
      << "isConnected" << connectInfo.isconnected()
      << "isReciveNoti" << connectInfo.isrecivenoti()
      << "serverName" << serverName_
      << "users" << users_array
      << "clientVersion" << connectInfo.clientversion()
      << "OSVersion" << connectInfo.osversion()
      << "appVersion" << connectInfo.appversion()
      << "timestamp" << ts.count()
      << finalize
      );
  }catch (std::system_error & e) {
    YILOG_ERROR ("connection insert error, uuid :{} ."
        "system_error code:{}, what:{}.", 
        connectInfo.uuid(), e.code().value(), e.what());
    throw std::system_error(std::error_code(40030, std::generic_category()),
        "connection insert failure");
  }

}
void mongo_client::updateUUID(const chat::ConnectInfo & connectInfo) {

  YILOG_TRACE ("func: {}. ", __func__);
  
  auto db = client_["chatdb"];
  auto connectinfo_col = db["connectInfo"];
  // session map {userid: sessionid}
  auto sessionbuilder = array{};
  for (auto & map: connectInfo.users()) {
    sessionbuilder << open_document
      << "userID" << map.first 
      << "sessionID" << map.second << close_document;
  }
  auto users_array = sessionbuilder << finalize;
  // insert
  try {
    auto now = std::chrono::high_resolution_clock::now();
    auto ts = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
  connectinfo_col.update_one(
      document{} << "UUID" << connectInfo.uuid()
      << finalize,
      document{} << "$set" << open_document
      << "userID" << connectInfo.userid()
      << "isLogin" << connectInfo.islogin()
      << "isConnected" << connectInfo.isconnected()
      << "isReciveNoti" << connectInfo.isrecivenoti()
      << "serverName" << serverName_
      << "users" << users_array 
      << "clientVersion" << connectInfo.clientversion()
      << "OSVersion" << connectInfo.osversion()
      << "appVersion" << connectInfo.appversion() 
      << "timestamp" << ts.count() << close_document
      << finalize
      );
  }catch (std::system_error & e) {
    YILOG_ERROR ("update error, uuid :{} ."
        "system_error code:{}, what:{}.", 
        connectInfo.uuid(), e.code().value(), e.what());
    throw std::system_error(std::error_code(40031, std::generic_category()),
        "connection update failure");
  }
}
void mongo_client::updateSessionID(const std::string & uuid,
      const std::string & userid,
      const uint16_t sessionid) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto connectinfo_col = db["connectInfo"];
  try {
  connectinfo_col.update_one(
      document{} << "UUID" << uuid
      << "users.userID" << userid
      << finalize,
      document{} << "$set" << open_document
      << "users.$.sessionID" << sessionid
      << close_document
      << finalize
      );
  }catch (std::system_error & e) {
    YILOG_ERROR ("update sessionid error, uuid :{} ."
        "system_error code:{}, what:{}.", 
        uuid, e.code().value(), e.what());
  }
}

bool mongo_client::findUUID(const std::string & uuid,
    chat::ConnectInfo & connectInfo) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto db = client_["chatdb"];
  auto connectinfo_col = db["connectInfo"];
  auto maybe_result = connectinfo_col.find_one(
      document{} << "UUID" << uuid << finalize);
  if (!maybe_result) {
    return false;
  }
  auto connectinfo = maybe_result->view();
  connectInfo.set_uuid(uuid);
  connectInfo.set_userid(connectinfo["userID"].
      get_utf8().value.to_string());
  connectInfo.set_islogin(connectinfo["isLogin"].get_bool().value);
  connectInfo.set_isconnected(connectinfo["isConnected"].
      get_bool().value);
  connectInfo.set_isrecivenoti(connectinfo["isReciveNoti"].
      get_bool().value);
  connectInfo.set_servername(serverName_);
  auto users = connectinfo["users"].get_array().value;
  connectInfo.clear_users();
  auto musers = connectInfo.mutable_users();
  for (auto it = users.begin();
      it != users.end();
      ++it) {
    auto doc = it->get_document().view();
    (*musers)[doc["userID"].get_utf8().value.to_string()] = 
      doc["sessionID"].get_int32().value;
  }
  return true;

}

//  user unread node
void mongo_client::insertUnreadNode(const std::string & userid,
                const std::string & tonodeid) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto userUnread = db["userUnread"];
  try {
    userUnread.insert_one(
        document{} << "userID" << userid
        << "toNodeID" << tonodeid
        << "unreadIncrement" << 1
        << "readedIncrement" << 1
        << finalize,
        journal_insert_);
  }catch(std::system_error & e) {
    YILOG_ERROR ("insert unread node failure.\n"
        "system_error code:{}, what{}.",
        e.code().value(), e.what());
  }
}
void mongo_client::updateReadedIncrement(const std::string & userid,
                           const std::string & tonodeid,
                           const int32_t readedIncrement) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto userUnread = db["userUnread"];
  try {
  userUnread.update_one(
      document{} << "userID" << userid
      << "toNodeID" << tonodeid
      << "readedIncrement" << open_document
      << "$lt" << readedIncrement << close_document
      << finalize,
      document{} << "$set" << open_document
      << "readedIncrement" << readedIncrement
      << close_document
      << finalize);
  }catch(std::system_error & e) {
    YILOG_ERROR ("update readed node failure.\n"
        "system_error code:{}, what{}.",
        e.code().value(), e.what());
  }
}
void mongo_client::updateUnreadIncrement(const std::string & userid,
                           const std::string & tonodeid,
                           const int32_t unreadIncrement) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto userUnread = db["userUnread"];
  try {
  userUnread.update_one(
      document{} << "userID" << userid
      << "toNodeID" << tonodeid
      << "unreadIncrement" << open_document
      << "$lt" << unreadIncrement << close_document
      << finalize,
      document{} << "$set" << open_document
      << "unreadIncrement" << unreadIncrement
      << close_document
      << finalize);
  }catch(std::system_error & e) {
    YILOG_ERROR ("update unread node failure.\n"
        "system_error code:{}, what{}.",
        e.code().value(), e.what());
  }
}
void mongo_client::unreadNodes(const std::string & userid,
   std::function<void(const std::string & tonodeid,
                      const int32_t unreadIncrement,
                      const int32_t readedIncrement)> && func) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto userUnread = db["userUnread"];
  auto cursor = userUnread.find(
      document{} << "userID" << userid
      << finalize);
  for (auto doc: cursor) {
    auto tonodeid = doc["toNodeID"].get_utf8().value.to_string();
    auto unreadIncrement = doc["unreadIncrement"].get_int32().value;
    auto readedIncrement = doc["readedIncrement"].get_int32().value;
    func(tonodeid, unreadIncrement, readedIncrement);
  }
}

namespace yijian{
  namespace threadCurrent {
    std::shared_ptr<mongo_client> mongoClient() {
      static thread_local auto client = 
        std::make_shared<mongo_client>(SERVER_NAME);
      return client;
    }
  }
}
