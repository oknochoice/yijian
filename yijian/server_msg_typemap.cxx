#include "server_msg_typemap.h"

#include <map>
#include <mutex>

#include "mongo.h"
#include "protofiles/chat_message.pb.h"
#include <string>
#include <queue>
#include "buffer_yi_util.hpp"
#include "libev_server.h"
#include <unordered_map>
#include <google/protobuf/util/json_util.h>
#include <memory>
#include <atomic>

#ifdef __cpluscplus
extern "C" {
#endif

//.cc
/* //hana library test
namespace hana = boost::hana;

template <typename T>
auto case_ = [](auto f) {
  return hana::make_pair(hana::type_c<T>, f);
};

struct default_t;
auto default_ = case_<default_t>;

template <typename Any, typename Default>
auto process(Any &, std::type_index const &, Default & default_) {
  return default_();
}
template <typename Any, typename Default, typename Case, typename ...Rest>
auto process(Any & a, std::type_index const & t, Default & default_,
            Case & case_, Rest & ...rest) {
  using T = typename decltype(+hana::first(case_))::type;
  if (t == typeid(T)) {
    return hana::second(case_)(*boost::unsafe_any_cast<T>(&a));
  }else {
    return process(a, t, default_, rest...);
  }
}

template <typename Any>
auto switch_(Any & a) {
  return [&a](auto ...cases_) {
    auto cases = hana::make_tuple(cases_...);

    auto default_ = hana::find_if(cases, [](auto const & c) {
        return hana::first(c) == hana::type_c<default_t>;
        });
    static_assert(default_ != hana::nothing, "switch is missing a default_ case");

    auto rest = hana::filter(cases, [](auto & c){
        return hana::first(c) != hana::type_c<default_t>;
        });
    return hana::unpack(rest, [&] (auto & ...rest) {
        return process(a, a.type(), hana::second(*default_), rest...);
        });
  };
}

template <typename Any>
auto dispatch(Any & a) {
  auto r = switch_(a)(
      case_<chat::Error>([](auto m) {
          std::cout << "chat error" << std::endl;
        }), 
      default_([]() {throw std::system("message type is not find");})
  );
  return r;
}
*/

using yijian::buffer;

namespace yijian {
  namespace threadCurrent {
    std::vector<Buffer_SP> errorBuffer(uint_fast32_t err_num, std::string && err_msg) {
      auto error = chat::Error();
      error.set_errnum(err_num);
      error.set_errmsg(err_msg);
      return yijianBuffer(error);
    }
  }
}

using yijian::threadCurrent::errorBuffer;
using yijian::threadCurrent::currentNode_;
using yijian::threadCurrent::connectInfo_;
using yijian::threadCurrent::node_self_;
using yijian::threadCurrent::node_peer_;
using yijian::threadCurrent::node_user_;
using yijian::threadCurrent::node_specifiy_;
using yijian::threadCurrent::infolittle_;
using yijian::threadCurrent::session_id_;

// user require device
void mountBuffer2Node(std::vector<Buffer_SP> && buf_sp_v, chat::NodeSelfDevice & ) {
  YILOG_TRACE ("func: {}. self device", __func__);
  std::unique_lock<std::mutex> ul(currentNode_->writeio_sp->buffers_p_mutex);
  for( auto sp: buf_sp_v) {
    sp->set_sessionid(session_id_, false);
    currentNode_->writeio_sp->buffers_p.push(sp);
  }
  buf_sp_v.back()->set_sessionid(session_id_, true);
  YILOG_TRACE ("func: {}. self device, write queue count {}", 
      __func__, currentNode_->writeio_sp->buffers_p.size());
  // push node
  yijian::threadCurrent::pushPingnode(currentNode_);
}

// peer server
void mountBuffer2Node(std::vector<Buffer_SP> && buf_sp_v, chat::NodePeerServer & ) {
  YILOG_TRACE ("func: {}. peer server", __func__);
  //transmit to peer server
  peer_server_foreach([&buf_sp_v](std::shared_ptr<Read_IO> lnode){
        // mount buffer to pingnode
        {
          std::unique_lock<std::mutex> ul(lnode->writeio_sp->buffers_p_mutex);
          for( auto sp: buf_sp_v) {
            sp->set_sessionid(session_id_, false);
            lnode->writeio_sp->buffers_p.push(sp);
          }
          buf_sp_v.back()->set_sessionid(session_id_, true);
          YILOG_TRACE ("func: {}. peer server, write queue count {}", 
              __func__, lnode->writeio_sp->buffers_p.size());
          // push node
          yijian::threadCurrent::pushPingnode(lnode);
        }
        // mount pingnode to thread data ,then stop read start write
        yijian::threadCurrent::pushPingnode(lnode);
          
      });
}

void traverseDevices(chat::ConnectInfoLittle & infolittle, std::vector<Buffer_SP> && buf_sp_v) {
  YILOG_TRACE ("func: {}. ", __func__);
  if (infolittle.isconnected()) {
    YILOG_TRACE ("func: {}. online", __func__);
    // get uuid
    std::string luuid = infolittle.uuid();
    // if node is request pass
    if (unlikely(currentNode_->uuid == luuid)) return;
    // mount buffer to pingnode
    YILOG_INFO ("luuid: {}", luuid);
    mountBuffer2Device(luuid, [&buf_sp_v](std::shared_ptr<Read_IO> io_sp){
          YILOG_INFO ("uuid: {}", io_sp->uuid);
          {
            std::unique_lock<std::mutex> uiol(
                io_sp->writeio_sp->buffers_p_mutex);
            for( auto sp: buf_sp_v) {
              sp->set_sessionid(session_id_, false);
              io_sp->writeio_sp->buffers_p.push(sp);
            }
            buf_sp_v.back()->set_sessionid(session_id_, true);
          }
          yijian::threadCurrent::pushPingnode(io_sp);
        });
  }
}

// current server subscribe to toNode devices(exclude require device)
void mountBuffer2Node(std::vector<Buffer_SP> && buf_sp_v, 
    chat::NodeSpecifiy & node_specifiy) {
  YILOG_TRACE ("func: {}. node specifiy", __func__);
  auto client = yijian::threadCurrent::mongoClient();
  auto node_sp = client->queryNode(node_specifiy.tonodeid());

  client->devices(node_sp->members(), currentNode_->userid,
      [&buf_sp_v](chat::ConnectInfoLittle & infolittle) {
        traverseDevices(infolittle, std::forward<std::vector<Buffer_SP>>(buf_sp_v));
      });
}

void mountBuffer2Node(std::vector<Buffer_SP> && buf_sp_v, chat::NodeUser & node_user) {
  YILOG_TRACE ("func: {}. node user: {}", 
      __func__, node_user.touserid());
  auto client = yijian::threadCurrent::mongoClient();
  client->devices(node_user, 
      [&buf_sp_v](chat::ConnectInfoLittle & infolittle) {
        traverseDevices(infolittle, std::forward<std::vector<Buffer_SP>>(buf_sp_v));
      });
}

// dispatch 
void dispatch(chat::Error& error) {

  YILOG_TRACE ("func: {}. ", __func__);

  if (unlikely(error.errnum() != 0)) {
    YILOG_ERROR ("error no {}, error message {}", 
        error.errnum(), error.errmsg());
  }

}

void dispatch(chat::Register & enroll) {
  
  YILOG_TRACE ("func: {}. register", __func__);

  YILOG_INFO ("register {}", pro2string(enroll));
  try {

    auto client = yijian::threadCurrent::mongoClient();
    if (enroll.verifycode().empty()) {
      // find user whether already signup
      auto user_sp = client->queryUser(enroll.phoneno(), enroll.countrycode());

      throw std::system_error(std::error_code(11007, std::generic_category()),
          "user had signup");
    }else {
      auto id = client->insertUser(enroll);

      YILOG_TRACE ("func: {}. register, userid {}.", 
          __func__, id);

      auto res = chat::RegisterRes();
      res.set_userid(id);
      res.set_issuccess(true);
      YILOG_INFO ("register success {}", pro2string(res));
      mountBuffer2Node(yijianBuffer(res), node_self_);
    }

  }catch (std::system_error & sys_error) {
    auto res = chat::RegisterRes();
    if (40002 == sys_error.code().value()) {
      res.set_issuccess(true);
      res.set_e_no(0);
      res.set_e_msg("not enroll");
    }else {
      res.set_issuccess(false);
      res.set_e_no(sys_error.code().value());
      res.set_e_msg(sys_error.what());
    }
    YILOG_INFO ("register failure {}", pro2string(res));
    mountBuffer2Node(yijianBuffer(res), node_self_);
  }

}

void dispatch(chat::Login & login) {

  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("login {}", pro2string(login));


  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto user_sp = client->queryUser(login.phoneno(), login.countrycode());
    if (user_sp->password() == login.password()) {
      // find device from db
      auto devices = user_sp->mutable_devices();
      auto it = find_if(devices->begin(), devices->end(),
                  [&](const chat::Device& device) -> bool {
                    return device.uuid() == login.device().uuid();
                  });
      // setup device
      auto device = std::make_shared<chat::Device>();
      device->set_os(login.device().os());
      device->set_devicemodel(login.device().devicemodel());
      device->set_uuid(login.device().uuid());
      device->set_devicenickname(login.device().devicenickname());
      // check whether finded
      if (it != devices->end()) {
        // update user
        client->updateDevice(user_sp->id(), *device);
      }else {
        client->insertDevice(user_sp->id(), *device);
      }
      // add global user connect info
      auto isFind = client->findUUID(device->uuid(), connectInfo_);
      YILOG_INFO ("connectInfo_: {}", pro2string(connectInfo_));
      if (isFind) {
        auto users_inconnectinfo = connectInfo_.mutable_users();
        auto it = users_inconnectinfo->find(user_sp->id());
        if (it == users_inconnectinfo->end()) {// not find
          (*users_inconnectinfo)[user_sp->id()] = MinSessionID;
        }else {
        }
        connectInfo_.set_uuid(device->uuid());
        connectInfo_.set_userid(user_sp->id());
        connectInfo_.set_islogin(true);
        connectInfo_.set_isconnected(false);
        connectInfo_.set_isrecivenoti(false);
        connectInfo_.set_servername(SERVER_NAME);

        client->updateUUID(connectInfo_);
      }else {
        connectInfo_.set_uuid(device->uuid());
        connectInfo_.set_userid(user_sp->id());
        connectInfo_.set_islogin(true);
        connectInfo_.set_isconnected(false);
        connectInfo_.set_isrecivenoti(false);
        connectInfo_.set_servername(SERVER_NAME);
        connectInfo_.clear_users();
        auto users_inconnectinfo = connectInfo_.mutable_users();
        (*users_inconnectinfo)[user_sp->id()] = MinSessionID;

        // insert connect info
        client->insertUUID(connectInfo_);
      }
      // record login
      //client->loginRecord(login.countrycode(), login.phoneno(), 
          //login.ips(), true);
      // response request device
      auto res = chat::LoginRes();
      res.set_issuccess(true);
      res.set_userid(user_sp->id());
      res.set_uuid(currentNode_->uuid);
      YILOG_INFO ("login success {}", pro2string(res));
      mountBuffer2Node(yijianBuffer(res), node_self_);
      node_user_.set_touserid(currentNode_->userid);

      auto noti = chat::LoginNoti();
      noti.set_uuid(login.device().uuid());
      YILOG_INFO ("send login noti {}", pro2string(noti));
      mountBuffer2Node(yijianBuffer(noti), node_user_);

      mountBuffer2Node(yijianBuffer(res), node_peer_);
    }else {
      //client->loginRecord(login.countrycode(), login.phoneno(), 
       //   login.ips(), false);
      throw std::system_error(std::error_code(11001, std::generic_category()),
          "password or account error");
    }

  }catch (std::system_error & sys_error) {
    auto res = chat::LoginRes();
    res.set_issuccess(false);
    res.set_e_no(sys_error.code().value());
    res.set_e_msg(sys_error.what());
    YILOG_INFO ("login failure {}", pro2string(res));
    mountBuffer2Node(yijianBuffer(res), node_self_);
  }

}

void dispatch(chat::LoginRes & res) {

  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("receive login noti {}", pro2string(res));
  try {
    auto noti = chat::LoginNoti();
    noti.set_uuid(res.uuid());
    node_user_.set_touserid(currentNode_->userid);
    mountBuffer2Node(yijianBuffer(noti), node_user_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::Logout & logout) {

  YILOG_TRACE ("func: {}. ", __func__);

  YILOG_INFO ("logout {}", pro2string(logout));

  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto user_sp = client->queryUser(logout.userid());
    // find device from db
    auto devices = user_sp->mutable_devices();
    auto it = find_if(devices->begin(), devices->end(),
                [&](const chat::Device& device) -> bool {
                  return device.uuid() == logout.uuid();
                });
    if (unlikely(it == devices->end())) {
      throw std::system_error(std::error_code(11002, std::generic_category()),
          "device not find");
    }else {
      // update connect info
      auto isFind = client->findUUID(it->uuid(), connectInfo_);
      YILOG_INFO ("connectInfo_: {}", pro2string(connectInfo_));
      if (unlikely(!isFind)) {
        YILOG_ERROR ("logout not find connection info");
      }
      connectInfo_.set_isconnected(false);
      connectInfo_.set_islogin(false);
      auto users_inconnectinfo = connectInfo_.mutable_users();
      (*users_inconnectinfo)[logout.uuid()] = session_id_;
      client->updateUUID(connectInfo_);

      // send buffer
      auto res = chat::LogoutRes();
      res.set_uuid(it->uuid());
      YILOG_INFO ("logout success {}", pro2string(res));
      mountBuffer2Node(yijianBuffer(res), node_self_);
      node_user_.set_touserid(currentNode_->userid);
    }

  }catch (std::system_error & sys_error) {
    YILOG_INFO ("logout failure");
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }


}


void dispatch(chat::ClientConnect & connect)  {

  YILOG_TRACE ("func: {}. ", __func__);

  YILOG_INFO ("connect {}", pro2string(connect));

  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto user_sp = client->queryUser(connect.userid());
    // find device from db
    auto devices = user_sp->mutable_devices();
    auto it = find_if(devices->begin(), devices->end(),
                [&](const chat::Device& device) -> bool {
                  return device.uuid() == connect.uuid();
                });
    if (unlikely(it == devices->end())) {
      throw std::system_error(std::error_code(11003, std::generic_category()),
          "device not find");
    }else {
      auto isFind = client->findUUID(connect.uuid(), connectInfo_);
      YILOG_INFO ("connectInfo_: {}", pro2string(connectInfo_));
      if (unlikely(!isFind)) {
        YILOG_ERROR ("connect not find connection info");
      }
      // set current node
      currentNode_->userid = connect.userid();
      currentNode_->uuid = connect.uuid();
      currentNode_->sessionid = connectInfo_.users().at(connect.userid());
      ++currentNode_->sessionid;
      currentNode_->clientVersion = connect.clientversion();
      currentNode_->appVersion = connect.appversion();
      // set current thread session id
      session_id_ = currentNode_->sessionid;
      // update in memory db connectinfo
      connectInfo_.set_isconnected(true);
      connectInfo_.set_isrecivenoti(connect.isrecivenoti());
      connectInfo_.set_clientversion(connect.clientversion());
      connectInfo_.set_appversion(connect.appversion());
      connectInfo_.set_osversion(connect.osversion());
      client->updateUUID(connectInfo_);
      // connect record
      //client->connectRecord(connect.userid(), connect.uuid(), connect.ips());
      // send buffer
      auto res = chat::ClientConnectRes();
      res.set_userid(connectInfo_.userid());
      res.set_issuccess(true);
      res.set_uuid(connectInfo_.uuid());
      res.set_sessionid(currentNode_->sessionid);
      YILOG_INFO ("connect success {}", pro2string(res));
      mountBuffer2Node(yijianBuffer(res), node_self_);
      // send unreaded 
      client->unreadNodes(currentNode_->userid,[]
          (const std::string & tonodeid,
           const int32_t unreadIncrement,
           const int32_t readedIncrement){
            auto noti = chat::NodeMessageNoti();
            noti.set_touserid_outer(currentNode_->userid);
            noti.set_tonodeid(tonodeid);
            noti.set_readedincrement(readedIncrement);
            noti.set_unreadincrement(unreadIncrement);
            mountBuffer2Node(yijianBuffer(noti), node_self_);
          });
    }
  }catch (std::system_error & sys_error) {
    auto res = chat::ClientConnectRes();
    res.set_issuccess(false);
    res.set_e_no(sys_error.code().value());
    res.set_e_msg(sys_error.what());
    YILOG_INFO ("connect failure {}", pro2string(res));
    mountBuffer2Node(yijianBuffer(res), node_self_);
  }

}

void dispatch(chat::ClientDisConnect & disconnect)  {

  YILOG_TRACE ("func: {}. ", __func__);

  YILOG_INFO ("disconnect {}", pro2string(disconnect));

  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto user_sp = client->queryUser(disconnect.userid());
    // find device from db
    auto devices = user_sp->mutable_devices();
    auto it = find_if(devices->begin(), devices->end(),
                [&](const chat::Device& device) -> bool {
                  return device.uuid() == disconnect.uuid();
                });
    if (unlikely(it == devices->end())) {
      throw std::system_error(std::error_code(11004, std::generic_category()),
          "device not find");
    }else {
      // update in memory db connectinfo
      auto isFind = client->findUUID(disconnect.uuid(), connectInfo_);
      YILOG_INFO ("connectInfo_: {}", pro2string(connectInfo_));
      if (unlikely(!isFind)) {
        YILOG_ERROR ("disconnect not find connection info");
      }
      connectInfo_.set_isconnected(false);
      auto users_inconnectinfo = connectInfo_.mutable_users();
      (*users_inconnectinfo)[disconnect.userid()] = session_id_;
      client->updateUUID(connectInfo_);
      // send buffer
      auto res = chat::ClientDisConnectRes();
      res.set_uuid(connectInfo_.uuid());
      YILOG_INFO ("disconnect success {}", pro2string(res));
      mountBuffer2Node(yijianBuffer(res), node_self_);
    }
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("disconnect failure");
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::AddFriend & frd) {

  YILOG_TRACE ("func: {}. ", __func__);

  YILOG_INFO ("add friend {}", pro2string(frd));

  try {
    if (unlikely(frd.inviterid().empty() || frd.inviteeid().empty()
        || frd.inviterid() != currentNode_->userid)) {
      throw std::system_error(std::error_code(11012, std::generic_category()),
          "args error");
    }
    auto client = yijian::threadCurrent::mongoClient();
    // check blacklist
    auto peerUser = client->queryUser(frd.inviteeid());
    auto it = find_if(peerUser->blacklist().begin(), peerUser->blacklist().end(),
        [&](const std::string & black) -> bool{
          return black == frd.inviteeid();
        });
    if (it != peerUser->blacklist().end()) {
      throw std::system_error(
          std::error_code(11005, std::generic_category()), 
          "blacklist");
    }
    // check whether is invter's friend
    auto user_sp = client->queryUser(frd.inviterid());
    auto & friends_inviter = user_sp->friends();
    auto user_it = find_if(friends_inviter.begin(), friends_inviter.end(),
        [&](const chat::UserInfo & info) -> bool {
          return info.userid() == frd.inviteeid();
        });
    if (unlikely(user_it != friends_inviter.end())) {
        throw std::system_error(std::error_code(11020, 
              std::generic_category()),
              "is friend already");
    }
    // check whether is invtee's friend
    auto & peer_friends = peerUser->friends();
    auto peer_it = find_if(peer_friends.begin(), peer_friends.end(),
        [&](const chat::UserInfo & info) -> bool {
          return info.userid() == frd.inviterid();
        });

    if (unlikely(peer_it != peer_friends.end())) {
        throw std::system_error(std::error_code(11022, 
              std::generic_category()),
              "is friend already");
    }
    // insert inviter invitee
    auto res = client->addFriend(frd);
    YILOG_INFO ("add friend success {}", pro2string(*res));
    // send buffer
    mountBuffer2Node(yijianBuffer(*res), node_self_);
    mountBuffer2Node(yijianBuffer(*res), node_peer_);

    // send to friend
    auto noti = chat::AddFriendNoti();
    auto addres = noti.mutable_response();
    *addres = *res;
    node_user_.set_touserid(res->inviteeid());
    YILOG_INFO ("add friend send noti to friend {}", pro2string(noti));
    mountBuffer2Node(yijianBuffer(noti), node_user_);
    // send buffer
    // send to other self
    node_user_.set_touserid(res->inviterid());
    YILOG_INFO ("add friend send noti to self {}", pro2string(noti));
    mountBuffer2Node(yijianBuffer(noti), node_user_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::AddFriendRes & res) {

  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("add friend noti {}", pro2string(res));

  try {
    // send to userid outer
    auto noti = chat::AddFriendNoti();
    *noti.mutable_response() = res;
    // invitee
    node_user_.set_touserid(res.inviteeid());
    mountBuffer2Node(yijianBuffer(noti), node_user_);
    // inviter
    node_user_.set_touserid(res.inviterid());
    mountBuffer2Node(yijianBuffer(noti), node_user_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::AddFriendAuthorize & addAuth) {
  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("add friend authorize {}", pro2string(addAuth));
  try {
    if (unlikely(addAuth.inviterid().empty() || addAuth.inviteeid().empty()
          || addAuth.inviteeid() != currentNode_->userid)) {
      throw std::system_error(std::error_code(11023, 
            std::generic_category()),
            "authorize permission error");
    }
    auto client = yijian::threadCurrent::mongoClient();
    auto authRes = chat::AddFriendAuthorizeRes();
    auto noti = chat::AddFriendAuthorizeNoti();
    if (addAuth.isagree() == chat::IsAgree::agree) {
      // add friend
      client->addFriendAuthorize(addAuth.inviterid(),
          addAuth.inviteeid());
      // set  
      authRes.set_inviterid(addAuth.inviterid());
      authRes.set_inviteeid(addAuth.inviteeid());
      authRes.set_isagree(chat::IsAgree::agree);
      *noti.mutable_response() = authRes;
      // self
      YILOG_INFO ("add friend authorize success {}", 
          pro2string(authRes));
      mountBuffer2Node(yijianBuffer(authRes), node_self_);
      // inviter
      node_user_.set_touserid(addAuth.inviterid());
      mountBuffer2Node(yijianBuffer(noti), node_user_);
      // invitee
      node_user_.set_touserid(addAuth.inviteeid());
      mountBuffer2Node(yijianBuffer(noti), node_user_);
      // peer
      mountBuffer2Node(yijianBuffer(authRes), node_peer_);
    }
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}

void dispatch(chat::AddFriendAuthorizeRes & res) {

  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("add friend authorize noti {}", pro2string(res));

  try {
    auto noti = chat::AddFriendAuthorizeNoti();
    *noti.mutable_response() = res;
    // inviter
    node_user_.set_touserid(res.inviterid());
    mountBuffer2Node(yijianBuffer(noti), node_user_);
    // invitee
    node_user_.set_touserid(res.inviteeid());
    mountBuffer2Node(yijianBuffer(noti), node_user_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}

void dispatch(chat::QueryAddfriendInfo & info) {
  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("query addfriend info {}", pro2string(info));
  try {
    auto client = yijian::threadCurrent::mongoClient();
    int limit = 10;
    if (info.count() > 0 && info.count() <= 50) {
      limit = info.count();
    }
    auto res = chat::QueryAddfriendInfoRes();
    client->queryAddfriendInfo([&res](chat::AddFriendInfo & info){
          auto info_p =res.add_info();
          *info_p = info;
        }, currentNode_->userid, limit);
    YILOG_INFO ("query addfriend info success {}", pro2string(res));
    mountBuffer2Node(yijianBuffer(res), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}

void dispatch(chat::CreateGroup & group) {
  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("create group {}", pro2string(group));
  try {
    auto client = yijian::threadCurrent::mongoClient();
    // filtrate black name 
    auto members = group.mutable_membersid();
    auto it = members->begin();
    while ( it != members->end()) {
      auto blacklist = client->queryUser(*it)->blacklist();
      auto black_it = std::find_if(blacklist.begin(), blacklist.end(),
          [&](const std::string & userid) {
            return userid == *it;
          });
      if (black_it != blacklist.end()) {
        ++it;
      }else {
        it = members->erase(it);
      }
    }
    if (unlikely(group.membersid().size() <= 3)) {
      throw std::system_error(std::error_code(11006, std::generic_category()),
          "at least 3 people");
    }
    // mongo insert group info
    auto groupRes = client->createGroup(group);
    YILOG_INFO ("create group success {}", pro2string(*groupRes));
    // send to self
    mountBuffer2Node(yijianBuffer(*groupRes), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}

void dispatch(chat::GroupAddMember & groupMember) {
  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("add group members {}", pro2string(groupMember));
  try {
    auto client = yijian::threadCurrent::mongoClient();
    // filtrate black name 
    auto members = groupMember.mutable_membersid();
    auto it = members->begin();
    while ( it != members->end()) {
      auto blacklist = client->queryUser(*it)->blacklist();
      auto black_it = std::find_if(blacklist.begin(), blacklist.end(),
          [&](const std::string & userid) {
            return userid == *it;
          });
      if (black_it != blacklist.end()) {
        ++it;
      }else {
        it = members->erase(it);
      }
    }
    // mongo insert group info
    auto addRes = client->addMembers2Group(groupMember);
    YILOG_INFO ("add group members success {}", 
        pro2string(*addRes));
    // send to self
    mountBuffer2Node(yijianBuffer(*addRes), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::QueryUserVersion & queryVersion) {
  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("query user version {}", pro2string(queryVersion));
  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto user_sp = client->queryUser(queryVersion.userid());
    auto version = chat::QueryUserVersionRes();
    version.set_version(user_sp->version());
    version.set_userid(queryVersion.userid());
    YILOG_INFO ("query user version success {}", 
        pro2string(version));
    mountBuffer2Node(yijianBuffer(version), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }


}

void dispatch(chat::QueryUser & queryUser) {
  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("query user {}", pro2string(queryUser));
  try {
    auto client = yijian::threadCurrent::mongoClient();
    std::shared_ptr<chat::User> user_sp = nullptr;
    if (!queryUser.userid().empty()) {
      user_sp = client->queryUser(queryUser.userid());
    }else {
      user_sp = client->queryUser(queryUser.phoneno(),
          queryUser.countrycode());
    }
    user_sp->clear_password();
    auto queryUserRes = chat::QueryUserRes();
    auto queryuser = queryUserRes.mutable_user();
    if (user_sp->id() != currentNode_->userid) {
      user_sp->clear_friends();
      user_sp->clear_blacklist();
      user_sp->clear_groupnodeids();
      user_sp->clear_devices();
    }
    *queryuser = *user_sp;
    YILOG_INFO ("query success {}", pro2string(queryUserRes));
    mountBuffer2Node(yijianBuffer(queryUserRes), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::QueryNodeVersion & querynodev) {

  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("query node version {}", pro2string(querynodev));

  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto node_sp = client->queryNode(querynodev.tonodeid());
    auto version = chat::QueryNodeVersionRes();
    version.set_version(node_sp->version());
    YILOG_INFO ("query node version success {}", 
        pro2string(version));
    mountBuffer2Node(yijianBuffer(version), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}


void dispatch(chat::QueryNode & querynode) {

  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("query node {}", pro2string(querynode));

  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto node_sp = client->queryNode(querynode.tonodeid());
    auto querynoderes = chat::QueryNodeRes();
    auto node = querynoderes.mutable_node();
    *node = *node_sp;
    YILOG_INFO ("query node success {}", pro2string(querynoderes));
    mountBuffer2Node(yijianBuffer(querynoderes), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}


void dispatch(chat::NodeMessage & message) {

  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("message {}", pro2string(message));


  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto res = client->insertMessage(message);
    // send self
    YILOG_INFO ("message success {}", 
        pro2string(*res));
    mountBuffer2Node(yijianBuffer(*res), node_self_);
    
    // noti self server and res peer server
    auto noti = chat::NodeMessageNoti();
    noti.set_fromuserid(currentNode_->userid);
    noti.set_unreadincrement(res->incrementid());
    noti.set_tonodeid(message.tonodeid());

    res->set_tonodeid(message.tonodeid());
    res->set_fromuserid(currentNode_->userid);
    
    if (likely(message.touserid_outer().empty())) {// node
      // update unread
      auto node_sp = client->queryNode(message.tonodeid());
      client->updateUnreadIncrement(node_sp->members(), 
        currentNode_->userid, message.tonodeid(), res->incrementid());
      // send to node
      node_specifiy_.set_tonodeid(message.tonodeid());
      mountBuffer2Node(yijianBuffer(noti), node_specifiy_);
      mountBuffer2Node(yijianBuffer(*res), node_peer_);
    }else {// user
      noti.set_touserid_outer(message.touserid_outer());
      res->set_touserid_outer(message.touserid_outer());
      // to self other device
      node_user_.set_touserid(currentNode_->userid);
      mountBuffer2Node(yijianBuffer(noti), node_user_);
      mountBuffer2Node(yijianBuffer(*res), node_peer_);
      // update unread
      client->updateUnreadIncrement(message.touserid_outer(),
          message.tonodeid(), res->incrementid());
      // to friend
      node_user_.set_touserid(message.touserid_outer());
      mountBuffer2Node(yijianBuffer(noti), node_user_);
      mountBuffer2Node(yijianBuffer(*res), node_peer_);
    }
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::NodeMessageRes & msgres) {

  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("message noti {}", pro2string(msgres));

  try {

    auto noti = chat::NodeMessageNoti();
    noti.set_fromuserid(msgres.fromuserid());
    noti.set_touserid_outer(msgres.touserid_outer());
    noti.set_unreadincrement(msgres.incrementid());
    noti.set_tonodeid(msgres.tonodeid());

    if (noti.touserid_outer().empty()) {
      // sent to node
      node_specifiy_.set_tonodeid(noti.tonodeid());
      mountBuffer2Node(yijianBuffer(noti), node_specifiy_);
    }else {
      // to user
      node_user_.set_touserid(noti.touserid_outer());
      noti.clear_touserid_outer();
      mountBuffer2Node(yijianBuffer(noti), node_user_);
    }
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::QueryMessage & query) {
  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("query message {}", pro2string(query));
  try {
    if (unlikely((query.toincrementid() <= query.fromincrementid() &&
            query.toincrementid() != 0)
        || (query.toincrementid() - query.fromincrementid()) > 50)) {
      throw std::system_error(std::error_code(11008, std::generic_category()),
          "query is invalid");
    }
    auto client = yijian::threadCurrent::mongoClient();
    // update user's readed  
    client->updateReadedIncrement(currentNode_->userid, 
        query.tonodeid(), query.toincrementid() - 1);
    //if (unlikely(query.toincrementid() == query.fromincrementid())) {
    //  throw std::system_error(std::error_code(0, std::generic_category()),
    //      "just update node's readed");
    //} else {
      auto res = chat::QueryMessageRes();
      client->queryMessage(query, [&res](chat::NodeMessage & msg){
            auto msg_p = res.add_messages();
            *msg_p = msg;
          });
      YILOG_INFO ("query message success {}", pro2string(res));
      mountBuffer2Node(yijianBuffer(res), node_self_);
    //}
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}

void dispatch(chat::QueryOneMessage & query) {
  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_INFO ("query one message {}", pro2string(query));
  try {
    auto client = yijian::threadCurrent::mongoClient();
    // update user's readed  
    //client->updateReadedIncrement(currentNode_->userid, 
    //    query.tonodeid(), query.incrementid());
    // query message
    auto nodemessage_sp = 
      client->queryMessage(query.tonodeid(), query.incrementid());
    auto res = chat::QueryMessageRes();
    auto msg = res.add_messages();
    *msg = *nodemessage_sp;
    YILOG_INFO ("query one message success {}", pro2string(res));
    mountBuffer2Node(yijianBuffer(res), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}

void dispatch(chat::Media & media) {
  YILOG_TRACE ("func: {}. media", __func__);
  YILOG_INFO ("media {}", pro2string(media));
  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto mediares = chat::MediaRes();
    client->insertMedia(media);
    mediares.set_issuccess(true);
    YILOG_INFO ("media {}", pro2string(mediares));
    mountBuffer2Node(yijianBuffer(mediares), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    if (sys_error.code().value() == 40061) {
      auto mediares = chat::MediaRes();
      mediares.set_issuccess(true);
      YILOG_INFO ("media {}", pro2string(mediares));
      mountBuffer2Node(yijianBuffer(mediares), node_self_);
    }else {
      mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
          node_self_);
    }
  }
}

void dispatch(chat::QueryMedia & querymedia) {
  YILOG_TRACE ("func: {}. media", __func__);
  YILOG_INFO ("query media {}", pro2string(querymedia));
  try {
    auto client = yijian::threadCurrent::mongoClient();
    int32_t maxlength = static_cast<int32_t>(Message_Type::message)
      - PADDING_LENGTH;
    auto res = chat::QueryMediaRes();
    auto media_p = res.mutable_media();
    client->queryMedia(querymedia.md5(), *media_p);
    YILOG_INFO ("query media success {}", pro2string(res));
    mountBuffer2Node(yijianBuffer(res), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}

void dispatch(chat::Ping & ping) {
  YILOG_TRACE ("func: {}. ping", __func__);
  YILOG_INFO ("ping {}", pro2string(ping));
  auto pong = chat::Pong();
  pong.set_msg("pong");
  /*
#ifdef YILOG_ON
  pong.set_msg("pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong"
               "pongpongpongpongpongpongpongpongpongpongpongpongpongpongpong");
#endif
*/
  mountBuffer2Node(yijianBuffer(pong), node_self_);
}

void dispatch(chat::SetUserProperty & property) {
  YILOG_TRACE ("func: {}. media", __func__);
  YILOG_INFO ("set property {}", pro2string(property));
  try {
    auto client = yijian::threadCurrent::mongoClient();
    client->setUserProperty(currentNode_->userid, property);
    auto res = chat::SetUserPropertyRes();
    res.set_property(property.property());
    res.set_value(property.value());
    mountBuffer2Node(yijianBuffer(res), node_self_);
  }catch (std::system_error & sys_error) {
    YILOG_INFO ("func: {}. failure. errno:{}, msg:{}.", 
        __func__, sys_error.code().value(), sys_error.what());
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}

void dispatch(const int type, char const * header, const std::size_t length) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto static map_p = std::make_shared<std::map<int, std::function<void(void)>>>();
  std::once_flag flag;
  std::call_once(flag, [&]() {
      (*map_p)[ChatType::error] = [=]() {
        auto chat = chat::Error();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::registor] = [=]() {
        auto chat = chat::Register();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::login] = [=]() {
        auto chat = chat::Login();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::loginres] = [=]() {
        auto chat = chat::LoginRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::logout] = [=]() {
        auto chat = chat::Logout();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::clientconnect] = [=]() {
        auto chat = chat::ClientConnect();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::clientdisconnect] = [=]() {
        auto chat = chat::ClientDisConnect();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::queryuser] = [=]() {
        auto chat = chat::QueryUser();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::queryuserversion] = [=]() {
        auto chat = chat::QueryUserVersion();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::addfriend] = [=]() {
        auto chat = chat::AddFriend();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::addfriendres] = [=]() {
        auto chat = chat::AddFriendRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::addfriendauthorize] = [=]() {
        auto chat = chat::AddFriendAuthorize();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::addfriendauthorizeres] = [=]() {
        auto chat = chat::AddFriendAuthorizeRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::creategroup] = [=]() {
        auto chat = chat::CreateGroup();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::groupaddmember] = [=]() {
        auto chat = chat::GroupAddMember();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::querynodeversion] = [=]() {
        auto chat = chat::QueryNodeVersion();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::querynode] = [=]() {
        auto chat = chat::QueryNode();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::nodemessage] = [=]() {
        auto chat = chat::NodeMessage();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::querymessage] = [=]() {
        auto chat = chat::QueryMessage();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::queryonemessage] = [=]() {
        auto chat = chat::QueryOneMessage();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::nodemessageres] = [=]() {
        auto chat = chat::NodeMessageRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::media] = [=]() {
        auto chat = chat::Media();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::querymedia] = [=]() {
        auto chat = chat::QueryMedia();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::queryaddfriendinfo] = [=]() {
        auto chat = chat::QueryAddfriendInfo();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::ping] = [=]() {
        auto chat = chat::Ping();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::setuserproterty] = [=]() {
        auto chat = chat::SetUserProperty();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
  });
  auto it = map_p->find(type);
  if (it != map_p->end()) {
    it->second();
  }else {
    YILOG_ERROR ("map_p not add type: {}.", type);
  }
}


void dispatch(const std::shared_ptr<Read_IO> node, 
    const std::vector<std::shared_ptr<yijian::buffer> > & spvec,
    const uint16_t session_id) {

  YILOG_TRACE ("func: dispatch tuple");
  currentNode_ = node;
  session_id_ = session_id;

  YILOG_DEBUG ("session id {}. sp session id: {}", 
      session_id_ ,  spvec.back()->session_id());
  YILOG_INFO ("pingtime:{}, isConnect:{}, sessionid:{}, "
      "userid:{}, uuid:{}", 
      currentNode_->ping_time, 
      currentNode_->isConnect, 
      currentNode_->sessionid, 
      currentNode_->userid, 
      currentNode_->uuid);

  std::string data_string;
  for (auto sp: spvec) {
    data_string.append(sp->data(), sp->data_size());
  }
  uint16_t data_sessionid = spvec.back()->session_id();
  uint8_t data_datatype = spvec.back()->datatype();

  // normal
  if (likely(session_id_ == data_sessionid)) {
    // miss msg check
    if (session_id_ >= MinSessionID) {
      if (likely(true == currentNode_->isConnect &&
            missing_check(data_datatype)
            )) {
        auto client = yijian::threadCurrent::mongoClient();
        client->updateSessionID(currentNode_->uuid, currentNode_->userid,
            session_id_);
        dispatch(data_datatype, data_string.c_str(), data_string.size());
      }else {
        auto error = chat::Error();
        error.set_errnum(11011);
        error.set_errmsg("need connect first or "
            "msg type can not pair the session id");
        mountBuffer2Node(yijianBuffer(error), node_self_);
      }
    }else {// miss msg no check
      if (likely(!missing_check(data_datatype))) {
        dispatch(data_datatype, data_string.c_str(), data_string.size());
      }else {
        auto error = chat::Error();
        error.set_errnum(11013);
        error.set_errmsg("session need < MinSessionID"
            "msg type can not pair the session id");
        mountBuffer2Node(yijianBuffer(error), node_self_);
      }
    }
  }else {// excaption
      auto error = chat::Error();
      error.set_errnum(11010);
      auto err_msg = "session id error, right id is " + 
        std::to_string(session_id_) + " .";
      error.set_errmsg(err_msg);
      mountBuffer2Node(yijianBuffer(error), node_self_);
  }
  currentNode_.reset();
}

#ifdef __cpluscplus
}
#endif

