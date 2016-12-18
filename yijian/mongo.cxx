#include "mongo.h"

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
  userinfo.set_userid(userinfo_view["userID"].
          get_utf8().value.to_string());

}

void deviceDocument(chat::Device & device,
    bsoncxx::document::view & device_view) {

  device.set_os(static_cast<chat::Device::OperatingSystem>
      (device_view["OS"].get_int32().value));
  device.set_devicemodel(device_view["deviceModel"].
      get_utf8().value.to_string());
  device.set_devicenickname(device_view["deviceNickname"].
      get_utf8().value.to_string());
  device.set_uuid(device_view["UUID"].get_utf8().value.to_string());

}

void messagenodeDocument(chat::MessageNode & messagenode,
    bsoncxx::document::view & messagenode_view) {
  messagenode.set_id(messagenode_view["_id"].get_oid().value.to_string());
  messagenode.set_authorize(
      static_cast<chat::MessageNode::Authorize>(
      messagenode_view["authorize"].get_int32().value));
  messagenode.set_creatorid(messagenode_view["creatorID"].
      get_utf8().value.to_string());
  messagenode.set_nickname(messagenode_view["nickname"].
      get_utf8().value.to_string());
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

void friendreqDocument(chat::FriendRequest & frdReq, 
    bsoncxx::document::view & view) {
  frdReq.set_isinviter(view["isInviter"].get_bool().value);
  frdReq.set_userid(view["userID"].get_utf8().value.to_string());
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

    auto friendreqs = 
      user_view["friendreqs"].get_array().value;
    for (auto it = friendreqs.begin();
        it != friendreqs.end();
        ++it) {
      auto doc_view = it->get_document().view();
      auto friendreq = user_sp->mutable_friendreqs()->Add();
      friendreqDocument(*friendreq, doc_view);
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

static thread_local unsigned char sha1_result[SHA_DIGEST_LENGTH];

mongo_client::mongo_client() {
  YILOG_TRACE ("func: {}. ", __func__);
  mongocxx::uri uri("mongodb://localhost:27017");
  client_ = mongocxx::client(uri);
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
    << "isMale" << ""
    << "code_phone" << enroll.countrycode() + "_" + enroll.phoneno()
    << "password" << enroll.password()
    << "birthday" << 0
    << "version" << "1"
    << "friends" << open_array << close_array
    << "blacklist" << open_array << close_array
    << "groupNodeIDs" << open_array << close_array
    << "devices" << open_array << close_array
    << "friendreqs" << open_array << close_array
    << finalize;
  auto maybe_result = user_collection.insert_one(doc_value.view());
  if (unlikely(!maybe_result)) {
    YILOG_ERROR ("insert user failure, countrycode:{}, phone .", 
        enroll.countrycode(), enroll.phoneno());
    throw std::system_error(std::error_code(40011, std::generic_category()),
        "insert user failure");
  }

  static thread_local std::string id;
  id.clear();
  id = maybe_result->inserted_id().get_oid().value.to_string();
  YILOG_TRACE ("func: {}. id {}", __func__, id);
  return id;
}

void mongo_client::updateDevice(
    const std::string & userID, const chat::Device & device) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  auto maybe_result = user_collection.update_one(
      document{} << "_id" << bsoncxx::oid(userID)
      << "devices.UUID" << device.uuid() 
      << finalize,
      document{} << "$set" << open_document 
      << "devices.$" << open_document
      << "OS" << device.os()
      << "deviceModel" << device.devicemodel()
      << "UUID" << device.uuid()
      << close_document << close_document
      << finalize
      );
  if (unlikely(!maybe_result)) {
    YILOG_ERROR ("update user device failure, uuid :{} .", device.uuid());
    throw std::system_error(std::error_code(40000, std::generic_category()),
        "update user device failure");
  }
}
void mongo_client::insertDevice(
    const std::string & userID, const chat::Device & device) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto db = client_["chatdb"];
  auto user_collection = db["user"];
  auto maybe_result = user_collection.update_one(
      document{} << "_id" << bsoncxx::oid(userID)
      << finalize,
      document{} << "$push" << open_document 
      << "devices" << open_document
      << "OS" << device.os()
      << "deviceModel" << device.devicemodel()
      << "UUID" << device.uuid()
      << close_document << close_document
      << finalize
      );
  if (unlikely(!maybe_result)) {
    YILOG_ERROR ("insert user device failure, uuid :{} .", device.uuid());
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
      document{} << "code_phone" << countryCode + "_" + phoneNo
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

  // create new node
  auto messageNode_collection = db["messageNode"];
  auto msgNode_maybe_result = messageNode_collection.insert_one(
      document{} << "authorize" << chat::MessageNode::peer
      << "creatorID" << addfrd.inviterid()
      << "nickname" << ""
      << "managerIDs" << open_array << close_array
      << "memebers" << open_array 
      << addfrd.inviterid() << addfrd.inviteeid() << close_array
      << finalize);
  if (unlikely(!msgNode_maybe_result)) {
    YILOG_ERROR("create message node failure");
    throw std::system_error(std::error_code(40004, std::generic_category()),
        "create message node failure");
  }
  auto msgNodeid = 
      msgNode_maybe_result->inserted_id().get_oid().value.to_string();
  // create messge node count
  auto messageNode_count_col = db["messageNodeCount"];
  auto count_maybe_result = messageNode_collection.insert_one(
      document{} << "nodeID" << msgNodeid
      << "count" << 0
      << finalize);
  if (unlikely(!count_maybe_result)) {
    YILOG_ERROR("create message node count failure, message node id: {}",
        msgNodeid);
    throw std::system_error(std::error_code(40005, std::generic_category()),
        "create message node count failure");
  }
  // add message node to inviter
  auto user_collection = db["user"];
  auto inviter_maybe_result = user_collection.update_one(
      document{} << "_id" << bsoncxx::oid(addfrd.inviterid())
      << finalize,
      document{} << "$addToSet" << open_document
      << "friendreqs" << open_document
      << "toNodeID" << msgNodeid 
      << "userID" << addfrd.inviteeid()
      << "isInviter" << true
      << "isAgree" << chat::IsAgree::ignore
      << close_document 
      << "$inc" << "version"
      << close_document
      << finalize);
  if (unlikely(!inviter_maybe_result)) {
    YILOG_ERROR ("add friendreqs failure, user id {}", addfrd.inviterid());
    throw std::system_error(std::error_code(40006, std::generic_category()),
        "add friendreqs failure");
  }
  // add message node to invitee
  auto invitee_maybe_result = user_collection.update_one(
      document{} << "_id" << bsoncxx::oid(addfrd.inviteeid())
      << finalize,
      document{} << "$addToSet" << open_document
      << "friendreqs" << open_document
      << "toNodeID" << msgNodeid 
      << "userID" << addfrd.inviterid()
      << "isInviter" << false
      << "isAgree" << chat::IsAgree::ignore
      << close_document 
      << "$inc" << "version"
      << close_document
      << finalize);
  if (unlikely(!inviter_maybe_result)) {
    YILOG_ERROR ("add friendreqs failure, user id {}", addfrd.inviteeid());
    throw std::system_error(std::error_code(40007, std::generic_category()),
        "add friendreqs failure");
  }

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

}

void mongo_client::addFriendAuthorize(const std::string & inviter, 
      const std::string & invitee, 
      const std::string & tonodeid) {
  YILOG_TRACE ("func: {}. ", __func__);

  auto db = client_["chatdb"];
  auto user_collection = db["user"];

  auto inviter_result = user_collection.update_one(
      document{} << "_id" << bsoncxx::oid(inviter)
      << finalize,
      document{} << "$addToSet" << open_document 
      << "friends" << open_document
      << "toNodeID" << tonodeid
      << "userID" << invitee << close_document
      << "$inc" << "version"
      << "$pull" << open_document
      << "friendreqs" << open_document
      << "toNodeID" << tonodeid
      << close_document << close_document
      << close_document << finalize);
  if (unlikely(!inviter_result)) {
    YILOG_ERROR ("add friend authorize failure, userid {}"
        "add userid {}",
        inviter, invitee);
    throw std::system_error(std::error_code(40008, std::generic_category()),
        "add friend authorize failure");
  }
  auto invitee_result = user_collection.update_one(
      document{} << "_id" << bsoncxx::oid(invitee)
      << finalize,
      document{} << "$addToSet" << open_document 
      << "friends" << open_document
      << "toNodeID" << tonodeid
      << "userID" << inviter << close_document
      << "$inc" << "version"
      << "$pull" << open_document
      << "friendreqs" << open_document
      << "toNodeID" << tonodeid
      << close_document << close_document
      << close_document << finalize);
  if (unlikely(!invitee_result)) {
    YILOG_ERROR ("add friend authorize failure, userid {}"
        "add userid {}",
        invitee, inviter);
    throw std::system_error(std::error_code(40009, std::generic_category()),
        "add friend authorize failure");
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
  // create message node
  auto maybe_result = messageNode_collection.insert_one(
      document{} << "authorize" << chat::MessageNode::peer
      << "creatorID" << cGroup.userid()
      << "nickname" << cGroup.nickname() 
      << "managerIDs" << open_array << close_array
      << "memebers" << memebers 
      << finalize);
  if (likely(maybe_result)) {
    auto obj_id = maybe_result->inserted_id().get_oid().value.to_string();
    // add message node count
    auto messageNode_count_col = db["messageNodeCount"];
    auto maybe_result_count = messageNode_collection.insert_one(
        document{} << "nodeID" << obj_id
        << "count" << 0
        << finalize);

    if (unlikely(!maybe_result_count)) {
      YILOG_ERROR("insert messageNodeCount failure, nodeID {}"
          , obj_id);
      throw std::system_error(std::error_code(40020, std::generic_category()),
          "insert message node count failure");
    }
    auto re = std::make_shared<chat::CreateGroupRes>();
    re->set_tonodeid(obj_id);
    re->set_nickname(cGroup.nickname());
    return re;
  }else {
    YILOG_ERROR("create message node failure, creater id", cGroup.userid());
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
  auto updateNode_result = messageNode_collection.update_one(
      document{} << "_id" << bsoncxx::oid(groupMember.groupnodeid())
      << finalize,
      document{} << "$addToSet" << open_document
      << "membersid" << open_document
      << "$each" << membersid << close_document << close_document
      << finalize);

  if (unlikely(!updateNode_result)) {
    YILOG_ERROR ("add group member failure, groupid: {}", 
        groupMember.groupnodeid());
    throw std::system_error(std::error_code(40022, std::generic_category()),
        "add group member failure");

  }
  auto addGroupRes = std::make_shared<chat::GroupAddMemberRes>();
  addGroupRes->set_groupnodeid(groupMember.groupnodeid());
  return addGroupRes;
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
  auto node_count = messageNode_count_col.find_one_and_update(
      document{} << "nodeID" << message.tonodeid()
      << finalize,
      document{} << "$inc" << open_document
      << "count" << 1 << close_document
      << finalize);
  if (likely(node_count)) {
    int32_t incrementid = node_count->view()["count"].get_int32().value;
    auto maybe_result = nodemessage_collection.insert_one(
        document{} << "fromUserID" << message.fromuserid()
        << "toNodeID" << message.tonodeid()
        << "incrementID" << incrementid
        << "type" << message.type()
        << "content" << message.content()
        << finalize);
    if (unlikely(!maybe_result)) {
      YILOG_ERROR("inc nodeID count success nodeid:{}" 
          "insert nodeid message failure, message's incrementid:{}", 
          message.tonodeid(), incrementid);
      throw std::system_error(std::error_code(40050, std::generic_category()),
          "insert message failure");
    }
    auto msgRes = std::make_shared<chat::NodeMessageRes>();
    msgRes->set_tonodeid(message.tonodeid());
    msgRes->set_incrementid(incrementid);
    return msgRes;
  }else {
    YILOG_ERROR("inc nodeID {} count failure", message.tonodeid());
    throw std::system_error(std::error_code(40051, std::generic_category()),
        "inc nodeID count failure");
  }
}

std::shared_ptr<chat::NodeMessage>
mongo_client::queryMessage(std::string & tonodeid, int32_t incrementid) {
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

std::shared_ptr<mongocxx::cursor>
mongo_client::cursor(chat::QueryMessage & query) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto db = client_["chatdb"];
  auto nodemessage_collection = db["nodeMessage"];
  auto builder = document{};
  builder << "toNodeID" << query.tonodeid();
  if (query.toincrementid() != 0) {
    builder << "incrementID" << open_document
      << "$gt" << query.fromincrementid() << close_document;
  }else {
    builder << "$and" << open_array << open_document
      << "incrementID" << open_document 
      << "$gt" << query.fromincrementid() 
      << close_document << close_document << open_document
      << "incrementID" << open_document
      << "$lt" << query.toincrementid()
      << close_document << close_document << close_array;
  }
  auto filter = builder << finalize;
  auto opt = mongocxx::options::find{};
  opt.sort(document{} << "incrementID" << -1 << finalize);
  auto maybe_result = nodemessage_collection.find(filter.view(), opt);
  
  return std::make_shared<mongocxx::cursor>(
      std::forward<mongocxx::cursor>(maybe_result));
}


std::shared_ptr<chat::NodeMessage>
mongo_client::queryMessage(std::shared_ptr<mongocxx::cursor> cursor_sp) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto message_sp = std::make_shared<chat::NodeMessage>();
  auto view = *cursor_sp->begin();
  nodemessageDocument(*message_sp, view);
  return message_sp;
  
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
  auto maybe_result = media_col.insert_one(
      document{} << "sha1" << sha1
      << "type" << media_vec.front().type()
      << "content" << content
      << finalize);
  if (unlikely(!maybe_result)) {
    YILOG_ERROR ("media insert failure, sha1: {}", sha1);
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


inmem_client::inmem_client(std::string serverName)
  : serverName_(serverName){
  mongocxx::uri uri("mongodb://localhost:27017");
  client_ = mongocxx::client(uri);
}


inmem_client::~inmem_client () {
}


void inmem_client::devices(const chat::NodeUser & node_user, 
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

void inmem_client::insertUUID(
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
  auto maybe_result = connectinfo_col.insert_one(
      document{} << "UUID" << connectInfo.uuid()
      << "userID" << connectInfo.userid()
      << "isLogin" << connectInfo.islogin()
      << "isConnected" << connectInfo.isconnected()
      << "isReciveNoti" << connectInfo.isrecivenoti()
      << "serverName" << serverName_
      << "nodepointor" << connectInfo.nodepointor()
      << "users" << users_array
      << "clientVersion" << connectInfo.clientversion()
      << "OSVersion" << connectInfo.osversion()
      << "appVersion" << connectInfo.appversion()
      << "timestamp" << 0
      << finalize
      );
  if (unlikely(!maybe_result)) {
    YILOG_ERROR ("connection insert error, uuid :{} .", connectInfo.uuid());
    throw std::system_error(std::error_code(40030, std::generic_category()),
        "connection insert failure");
  }

}
void inmem_client::updateUUID(const chat::ConnectInfo & connectInfo) {

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
  auto maybe_result = connectinfo_col.update_one(
      document{} << "UUID" << connectInfo.uuid()
      << finalize,
      document{} << "$set" << open_document
      << "userID" << connectInfo.userid()
      << "isLogin" << connectInfo.islogin()
      << "isConnected" << connectInfo.isconnected()
      << "isReciveNoti" << connectInfo.isrecivenoti()
      << "serverName" << serverName_
      << "nodepointor" << connectInfo.nodepointor()
      << "users" << users_array 
      << "clientVersion" << connectInfo.clientversion()
      << "OSVersion" << connectInfo.osversion()
      << "appVersion" << connectInfo.appversion() << close_document
      << "currentDate" << open_document 
      << "timestamp" << open_document 
      << "$type" << "timestamp" << close_document << close_document
      << finalize
      );
  if (unlikely(!maybe_result)) {
    YILOG_ERROR ("update error, uuid :{} .", connectInfo.uuid());
    throw std::system_error(std::error_code(40031, std::generic_category()),
        "connection update failure");
  }
}

bool inmem_client::findUUID(const std::string & uuid,
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
  connectInfo.set_nodepointor(connectinfo["nodepointor"].
      get_int64().value);
  auto users = connectinfo["users"].get_array().value;
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

