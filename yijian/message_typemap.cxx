#include "message_typemap.h"

#include <map>
#include <mutex>

#include "mongo.h"
#include "protofiles/chat_message.pb.h"
#include "threads_work.h"
#include <string>
#include <queue>
#include "buffer.h"
#include "libev_server.h"
#include "pinglist.h"
#include "macro.h"


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

template <typename Proto> Buffer_SP
encoding(Proto any) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto buf = std::make_shared<yijian::buffer>();
  buf->data_encoding_length(any.ByteSize());
  buf->data_encoding_type(dispatchType(any));
  any.SerializeToArray(buf->data_encoding_current(), buf->remain_size());
  buf->data_encoding_reset_size(any.ByteSize());

  return buf;
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
    Buffer_SP errorBuffer() {
      static thread_local auto error = chat::Error();
      error.set_errnum(0);
      error.set_errmsg("success");
      return encoding(error);
    }
    Buffer_SP errorBuffer(uint_fast32_t err_num, std::string && err_msg) {
      auto error = chat::Error();
      error.set_errnum(err_num);
      error.set_errmsg(err_msg);
      return encoding(std::forward<std::string>(err_msg));
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

template <typename Any>
void mountBuffer2Node(Any &) {
  YILOG_TRACE ("func: {}. ", __func__);
  throw std::system_error(std::error_code(11007, std::generic_category()), 
      "unkonw node type");
}

// user require device
void mountBuffer2Node(Buffer_SP buf_sp, chat::NodeSelfDevice & ) {
  std::unique_lock<std::mutex> ul(currentNode_->buffers_p_mutex);
  currentNode_->contra_io->buffers_p.push(buf_sp);
}

// peer server
void mountBuffer2Node(Buffer_SP buf_sp, chat::NodePeerServer & ) {
  //transmit to peer server
  auto sp = peer_servers();
  for (auto & lnode: *sp) {
    // mount buffer to pingnode
    {
      std::unique_lock<std::mutex> ul(lnode->buffers_p_mutex);
      lnode->contra_io->buffers_p.push(buf_sp);
    }
    // mount pingnode to thread data ,then stop read start write
    yijian::threadCurrent::pushPingnode(lnode);
  }
}

void traverseDevices(chat::ConnectInfoLittle & infolittle, Buffer_SP buf_sp) {
  if (infolittle.islogin()) {
    if (infolittle.isconnected()) {
      YILOG_TRACE ("online");
      // get pingnode
      Pointor_t nodepointor = infolittle.nodepointor();
      PingNode * lnode = reinterpret_cast<PingNode*>(nodepointor);
      // if node is request pass
      if (unlikely(currentNode_ == lnode)) return;
      // mount buffer to pingnode
      {
        std::unique_lock<std::mutex> ul(lnode->buffers_p_mutex);
        lnode->contra_io->buffers_p.push(buf_sp);
      }
      // mount pingnode to thread data ,then stop read start write
      yijian::threadCurrent::pushPingnode(lnode);
    }else if (infolittle.isrecivenoti()) {
      YILOG_TRACE ("offline");
#warning need push server;
      std::cout << "push to " << infolittle.uuid() 
        << std::endl;
    }
  }
}

// current server subscribe to toNode devices(exclude require device)
void mountBuffer2Node(Buffer_SP buf_sp, chat::NodeSpecifiy & node_specifiy) {
  auto inClient = yijian::threadCurrent::inmemClient();
  inClient->devices(node_specifiy, [buf_sp](chat::ConnectInfoLittle & infolittle) {
        traverseDevices(infolittle, buf_sp);
      });
}

void mountBuffer2Node(Buffer_SP buf_sp, chat::NodeUser & node_user) {
  auto inClient = yijian::threadCurrent::inmemClient();
  inClient->devices(node_user, [buf_sp](chat::ConnectInfoLittle & infolittle) {
        traverseDevices(infolittle, buf_sp);
      });
}

template <typename Any>
void dispatch(Any & ) {
  YILOG_TRACE ("func: {}. ", __func__);
  throw std::system_error(std::error_code(11000, std::generic_category()), 
      "unkonw dispatch type");
}

constexpr uint8_t dispatchType(chat::Error & ) {
  return ChatType::error;
}
constexpr uint8_t dispatchType(chat::Register & ) {
  return ChatType::registor;
}
constexpr uint8_t dispatchType(chat::Login & ) {
  return ChatType::login;
}
constexpr uint8_t dispatchType(chat::LoginRes & ) {
  return ChatType::loginres;
}
constexpr uint8_t dispatchType(chat::Logout & ) {
  return ChatType::logout;
}
constexpr uint8_t dispatchType(chat::LogoutRes & ) {
  return ChatType::logoutres;
}
constexpr uint8_t dispatchType(chat::Connect & ) {
  return ChatType::connect;
}
constexpr uint8_t dispatchType(chat::ConnectRes & ) {
  return ChatType::connectres;
}
constexpr uint8_t dispatchType(chat::DisConnect & ) {
  return ChatType::disconnect;
}
constexpr uint8_t dispatchType(chat::DisConnectRes & ) {
  return ChatType::disconnectres;
}
constexpr uint8_t dispatchType(chat::QueryUser & ) {
  return ChatType::queryuser;
}
constexpr uint8_t dispatchType(chat::QueryUserRes & ) {
  return ChatType::queryuserres;
}
constexpr uint8_t dispatchType(chat::QueryUserVersion & ) {
  return ChatType::queryuserversion;
}
constexpr uint8_t dispatchType(chat::QueryUserVersionRes & ) {
  return ChatType::queryuserversionres;
}
constexpr uint8_t dispatchType(chat::AddFriend & ) {
  return ChatType::addfriend;
}
constexpr uint8_t dispatchType(chat::AddFriendRes & ) {
  return ChatType::addfriendres;
}
constexpr uint8_t dispatchType(chat::AddFriendAuthorize & ) {
  return ChatType::addfriendauthorize;
}
constexpr uint8_t dispatchType(chat::AddFriendAuthorizeRes & ) {
  return ChatType::addfriendres;
}
constexpr uint8_t dispatchType(chat::CreateGroup & ) {
  return ChatType::creategroup;
}
constexpr uint8_t dispatchType(chat::CreateGroupRes & ) {
  return ChatType::creategroupres;
}
constexpr uint8_t dispatchType(chat::GroupAddMember & ) {
  return ChatType::groupaddmember;
}
constexpr uint8_t dispatchType(chat::GroupAddMemberRes & ) {
  return ChatType::groupaddmemberres;
}
constexpr uint8_t dispatchType(chat::NodeMessage & ) {
  return ChatType::nodemessage;
}
constexpr uint8_t dispatchType(chat::NodeMessageRes & ) {
  return ChatType::nodemessageres;
}
constexpr uint8_t dispatchType(chat::QueryOneMessage & ) {
  return ChatType::queryonemessage;
}
constexpr uint8_t dispatchType(chat::QueryMessage & ) {
  return ChatType::querymessage;
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
  
  YILOG_TRACE ("func: {}. ", __func__);


  try {

    auto client = yijian::threadCurrent::mongoClient();
    client->insertUser(enroll);

    mountBuffer2Node(errorBuffer(), node_self_);

  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::Login & login) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto client = yijian::threadCurrent::mongoClient();
  auto inmem_client = yijian::threadCurrent::inmemClient();

  try {
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
      device->set_osversion(login.device().osversion());
      device->set_clientversion(login.device().clientversion());
      device->set_appversion(login.device().appversion());
      device->set_devicemodel(login.device().devicemodel());
      device->set_uuid(login.device().uuid());
      // check whether finded
      if (it != devices->end()) {
        // update user
        client->updateDevice(user_sp->id(), *device);
      }else {
        client->insertDevice(user_sp->id(), *device);
      }
      // add global user connect info
      connectInfo_.set_uuid(device->uuid());
      connectInfo_.set_userid(user_sp->id());
      connectInfo_.set_islogin(login.islogin());
      connectInfo_.set_isconnected(login.isconnected());
      connectInfo_.set_isrecivenoti(login.isrecivenoti());
      connectInfo_.set_servername(SERVER_NAME);
      connectInfo_.set_nodepointor(reinterpret_cast<Pointor_t>(currentNode_));
      connectInfo_.clear_tonodeids();
      connectInfo_.add_tonodeids("0");
      // add new group connect info
      for (auto & groupnodeid: user_sp->groupnodeids()) {
        connectInfo_.add_tonodeids(groupnodeid);
      }
      // insert connect info
      inmem_client->insertConnectInfo(connectInfo_);
      // response request device
      auto res = chat::LoginRes();
      res.set_uuid(device->uuid());
      mountBuffer2Node(encoding(res), node_self_);
      res.set_userid(currentNode_->userid);
      mountBuffer2Node(encoding(res), node_peer_);
      node_user_.set_touserid(currentNode_->userid);
      mountBuffer2Node(encoding(res), node_user_);
      // set current node
      currentNode_->userid = user_sp->id();
      currentNode_->deviceid = login.device().uuid();
    }else {
      mountBuffer2Node(errorBuffer(11001, "password or account error"),
          node_self_);
    }

  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::LoginRes & res) {

  YILOG_TRACE ("func: {}. ", __func__);
  try {
    node_user_.set_touserid(res.userid());
    res.clear_userid();
    mountBuffer2Node(encoding(res), node_user_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::Logout & logout) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto client = yijian::threadCurrent::mongoClient();

  try {
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
      client->updateDevice(user_sp->id(), *it);
      // update connect info
      // remove device connect info
      auto inmem_client = yijian::threadCurrent::inmemClient();
      inmem_client->removeConnectInfo(it->uuid());
      // send buffer
      auto res = chat::LogoutRes();
      res.set_uuid(it->uuid());
      mountBuffer2Node(encoding(res), node_self_);
      node_user_.set_touserid(currentNode_->userid);
      res.set_userid(currentNode_->userid);
      mountBuffer2Node(encoding(res), node_user_);
      mountBuffer2Node(encoding(res), node_peer_);
    }

  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }


}


void dispatch(chat::LogoutRes & res) {
  try {
    node_user_.set_touserid(res.userid());
    res.clear_userid();
    mountBuffer2Node(encoding(res), node_user_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}
void dispatch(chat::Connect & connect)  {

  YILOG_TRACE ("func: {}. ", __func__);

  auto client = yijian::threadCurrent::mongoClient();

  try {
    auto user_sp = client->queryUser(connect.userid());
    // find device from db
    auto devices = user_sp->mutable_devices();
    auto it = find_if(devices->begin(), devices->end(),
                [&](const chat::Device& device) -> bool {
                  return device.uuid() == connect.uuid();
                });
    if (unlikely(it == devices->end())) {
      mountBuffer2Node(errorBuffer(11003, "device not find"), 
          node_self_);
    }else {
      // set current node
      currentNode_->userid = connect.userid();
      currentNode_->deviceid = connect.uuid();
      // update in memory db connectinfo
      // add new group connect info
      infolittle_.set_uuid(it->uuid());
      infolittle_.set_islogin(true);
      infolittle_.set_isconnected(true);
      infolittle_.set_isrecivenoti(connect.isrecivenoti());
      infolittle_.set_nodepointor(reinterpret_cast<Pointor_t>(currentNode_));
      auto inmem_client = yijian::threadCurrent::inmemClient();
      inmem_client->updateConnectInfo(infolittle_);
      // send buffer
      auto res = chat::ConnectRes();
      res.set_uuid(connectInfo_.uuid());
      mountBuffer2Node(encoding(res), node_self_);
    }
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::DisConnect & disconnect)  {

  YILOG_TRACE ("func: {}. ", __func__);

  auto client = yijian::threadCurrent::mongoClient();

  try {
    auto user_sp = client->queryUser(disconnect.userid());
    // find device from db
    auto devices = user_sp->mutable_devices();
    auto it = find_if(devices->begin(), devices->end(),
                [&](const chat::Device& device) -> bool {
                  return device.uuid() == disconnect.uuid();
                });
    if (unlikely(it == devices->end())) {
      mountBuffer2Node(errorBuffer(11004, "device not find"),
          node_self_);
    }else {
      // update in memory db connectinfo
      auto inmem_client = yijian::threadCurrent::inmemClient();
      inmem_client->disconnectInfo(disconnect.uuid());
      // send buffer
      auto res = chat::DisConnectRes();
      res.set_uuid(connectInfo_.uuid());
      mountBuffer2Node(encoding(res), node_self_);
    }
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::AddFriend & frd) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto client = yijian::threadCurrent::mongoClient();

  try {
    // check blacklist
    auto peerUser = client->queryUser(frd.inviteeid());
    find_if(peerUser->blacklist().begin(), peerUser->blacklist().end(),
        [&](const std::string & black) -> bool{
          bool re = black == frd.inviteeid();
          if (re) {
            throw std::system_error(
                std::error_code(11005, std::generic_category()), 
                "blacklist");
          }
          return re;
        });
    // insert inviter invitee
    auto res = std::make_shared<chat::AddFriendRes>();
    res->set_inviterid(currentNode_->userid);
    res->set_inviteeid(frd.inviteeid());
    res->set_msg(frd.msg());
    res->set_nickname(frd.nickname());
    // send buffer
    auto buf_sp = encoding(*res);
    mountBuffer2Node(buf_sp, node_self_);

    // send to friend
    node_user_.set_touserid(res->inviteeid());
    mountBuffer2Node(buf_sp, node_user_);
    mountBuffer2Node(encoding(*res), node_peer_);
    // send to other self
    node_user_.set_touserid(res->inviterid());
    mountBuffer2Node(buf_sp, node_user_);
    mountBuffer2Node(encoding(*res), node_peer_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::AddFriendRes & frdRes) {

  YILOG_TRACE ("func: {}. ", __func__);

  try {
    auto buf_sp = encoding(frdRes);
    // send to friend
    node_user_.set_touserid(frdRes.inviteeid());
    mountBuffer2Node(buf_sp, node_user_);
    // send to other self
    node_user_.set_touserid(frdRes.inviterid());
    mountBuffer2Node(buf_sp, node_user_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::AddFriendAuthorize & addAuth) {
  YILOG_TRACE ("func: {}. ", __func__);
  try {
    if (addAuth.isagree() == true) {
      // add friend
      auto client = yijian::threadCurrent::mongoClient();
      auto info = client->addFriendAuthorize(
          addAuth.inviterid(), addAuth.inviternickname(),
          addAuth.inviteeid(), addAuth.inviteenickname());
      // send 
      auto authRes = chat::AddFriendAuthorizeRes();
      authRes.set_isagree(true);
      // inviter
      authRes.mutable_friend_()->set_tonodeid(info->tonodeid());
      authRes.mutable_friend_()->set_nickname(addAuth.inviternickname());
      authRes.mutable_friend_()->set_userid(addAuth.inviterid());
      authRes.set_userversion(info->inviteruserversion());

      node_user_.set_touserid(addAuth.inviterid());
      auto inviter_buf = encoding(authRes);
      mountBuffer2Node(inviter_buf, node_user_);
      mountBuffer2Node(inviter_buf, node_peer_);
      // invitee
      authRes.mutable_friend_()->set_tonodeid(info->tonodeid());
      authRes.mutable_friend_()->set_nickname(addAuth.inviteenickname());
      authRes.mutable_friend_()->set_userid(addAuth.inviteeid());
      authRes.set_userversion(info->inviteeuserversion());

      node_user_.set_touserid(addAuth.inviteeid());
      auto invitee_buf = encoding(authRes);
      mountBuffer2Node(invitee_buf, node_self_);
      mountBuffer2Node(invitee_buf, node_user_);
      mountBuffer2Node(invitee_buf, node_peer_);
    }
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}

void dispatch(chat::AddFriendAuthorizeRes & authRes) {

  YILOG_TRACE ("func: {}. ", __func__);

  try {
    node_user_.set_touserid(authRes.friend_().userid());
    mountBuffer2Node(encoding(authRes), node_user_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}

void dispatch(chat::CreateGroup & group) {
  YILOG_TRACE ("func: {}. ", __func__);
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
    // connectinfo insert info
    auto inmem_client = yijian::threadCurrent::inmemClient();
    inmem_client->addTonodeidConnectInfo(
        group.membersid(), groupRes->tonodeid());
    // send to self
    mountBuffer2Node(encoding(groupRes), node_self_);
    // self other device
    node_user_.set_touserid(currentNode_->userid);
    mountBuffer2Node(encoding(groupRes), node_user_);
    // peer 
    groupRes->set_touserid_outer(currentNode_->userid);
    mountBuffer2Node(encoding(groupRes), node_peer_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}

void dispatch(chat::CreateGroupRes & groupRes) {
  YILOG_TRACE ("func: {}. ", __func__);
  try {
    node_user_.set_touserid(groupRes.touserid_outer());
    groupRes.clear_touserid_outer();
    mountBuffer2Node(encoding(groupRes), node_user_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::GroupAddMember & groupMember) {
  YILOG_TRACE ("func: {}. ", __func__);
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
    // connectinfo insert info
    auto inmem_client = yijian::threadCurrent::inmemClient();
    inmem_client->addTonodeidConnectInfo(
        groupMember.membersid(), groupMember.groupnodeid());
    // send to self
    mountBuffer2Node(encoding(*addRes), node_self_);
    // send to self other device
    node_user_.set_touserid(addRes->touserid_outer());
    mountBuffer2Node(encoding(*addRes), node_user_);
    // send to peer
    addRes->set_touserid_outer(currentNode_->userid);
    mountBuffer2Node(encoding(*addRes), node_peer_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::GroupAddMemberRes & addMemberRes) {

  YILOG_TRACE ("func: {}. ", __func__);
  try {
    node_user_.set_touserid(addMemberRes.touserid_outer());
    mountBuffer2Node(encoding(addMemberRes), node_user_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::QueryUserVersion & ) {
  YILOG_TRACE ("func: {}. ", __func__);
  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto user_sp = client->queryUser(currentNode_->userid);
    auto version = chat::QueryUserVersionRes();
    version.set_version(user_sp->version());
    mountBuffer2Node(encoding(version), node_self_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }


}

void dispatch(chat::QueryUser & queryUser) {
  YILOG_TRACE ("func: {}. ", __func__);
  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto user_sp = client->queryUser(queryUser.userid());
    user_sp->clear_password();
    auto queryUserRes = chat::QueryUserRes();
    auto queryuser = queryUserRes.mutable_user();
    if (queryUser.userid() != currentNode_->userid) {
      user_sp->clear_version();
      user_sp->clear_friends();
      user_sp->clear_blacklist();
      user_sp->clear_groupnodeids();
      user_sp->clear_devices();
    }
    *queryuser = *user_sp;
    mountBuffer2Node(encoding(queryUserRes), node_self_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::QueryNode & querynode) {

  YILOG_TRACE ("func: {}. ", __func__);

  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto node_sp = client->queryNode(querynode.tonodeid());
    auto querynoderes = chat::QueryNodeRes();
    auto node = querynoderes.mutable_node();
    *node = *node_sp;
    mountBuffer2Node(encoding(querynoderes), node_self_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}


void dispatch(chat::NodeMessage & message) {

  YILOG_TRACE ("func: {}. ", __func__);


  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto res = client->insertMessage(message);
    if (likely(message.touserid_outer().empty())) {// node
      auto encoding_res_sp = encoding(*res);
      mountBuffer2Node(encoding_res_sp, node_self_);
      mountBuffer2Node(encoding_res_sp, node_peer_);

      // send to node
      auto node_specifiy = chat::NodeSpecifiy();
      node_specifiy.set_tonodeid(res->tonodeid());
      mountBuffer2Node(encoding(message), node_specifiy);
    }else {// user
      // self
      mountBuffer2Node(encoding(*res), node_self_);
      // peer

      // to friend
      node_user_.set_touserid(message.touserid_outer());
      mountBuffer2Node(encoding(message), node_user_);
      res->set_touserid_outer(message.touserid_outer());
      mountBuffer2Node(encoding(*res), node_peer_);
      // to self other device
      node_user_.set_touserid(currentNode_->userid);
      mountBuffer2Node(encoding(message), node_user_);
      res->set_touserid_outer(currentNode_->userid);
      mountBuffer2Node(encoding(*res), node_peer_);
    }
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::NodeMessageRes & messageRes) {

  YILOG_TRACE ("func: {}. ", __func__);

  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto sp = client->queryMessage(messageRes);
    if (messageRes.touserid_outer().empty()) {
      // sent to node
      auto node_specifiy = chat::NodeSpecifiy();
      node_specifiy.set_tonodeid(sp->tonodeid());
      mountBuffer2Node(encoding(*sp), node_specifiy);
    }else {
      auto encoding_sp = encoding(*sp);
      // to user
      node_user_.set_touserid(messageRes.touserid_outer());
      mountBuffer2Node(encoding_sp, node_user_);
    }
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::QueryOneMessage & query) {
  YILOG_TRACE ("func: {}. ", __func__);
  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto sp = client->queryMessage(query);
    mountBuffer2Node(encoding(*sp), node_self_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}

void dispatch(chat::QueryMessage & query) {
  YILOG_TRACE ("func: {}. ", __func__);
  try {
    auto client = yijian::threadCurrent::mongoClient();
    if (!query.tonodeid().empty() &&
        currentNode_->id_cursor.first != query.tonodeid()) {
      currentNode_->id_cursor.first = query.tonodeid();
      currentNode_->id_cursor.second = client->cursor(query);
    }
    if (unlikely(currentNode_->id_cursor.first.empty() ||
          currentNode_->id_cursor.second == nullptr)) {
      throw std::system_error(std::error_code(11008, std::generic_category()),
          "query tonodeid is empty");
    }
    bool isStop = false;
    int  count = 0;
    do {
      auto nodemessage_sp = client->queryMessage(
          currentNode_->id_cursor.second);
      if (nodemessage_sp->incrementid() <= query.fromincrementid()) {
        isStop = true;
      }
      if (10 == count) {
        isStop = true;
      }
      ++count;
      mountBuffer2Node(encoding(*nodemessage_sp), node_self_);
    }while(isStop);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}

void dispatch(int type, char * header, std::size_t length) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto static map_p = new std::map<int, std::function<void(void)>>();
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
      (*map_p)[ChatType::logoutres] = [=]() {
        auto chat = chat::LogoutRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::connect] = [=]() {
        auto chat = chat::Connect();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::connectres] = [=]() {
        auto chat = chat::ConnectRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::disconnect] = [=]() {
        auto chat = chat::DisConnect();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::disconnectres] = [=]() {
        auto chat = chat::DisConnectRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::queryuser] = [=]() {
        auto chat = chat::QueryUser();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::queryuserres] = [=]() {
        auto chat = chat::QueryUserRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::queryuserversion] = [=]() {
        auto chat = chat::QueryUserVersion();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::queryuserversionres] = [=]() {
        auto chat = chat::QueryUserVersionRes();
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
      (*map_p)[ChatType::creategroupres] = [=]() {
        auto chat = chat::CreateGroupRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::groupaddmember] = [=]() {
        auto chat = chat::GroupAddMember();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::groupaddmemberres] = [=]() {
        auto chat = chat::GroupAddMemberRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::nodemessage] = [=]() {
        auto chat = chat::NodeMessage();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::nodemessageres] = [=]() {
        auto chat = chat::NodeMessageRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::queryonemessage] = [=]() {
        auto chat = chat::QueryOneMessage();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::querymessage] = [=]() {
        auto chat = chat::QueryMessage();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
  });
  (*map_p)[type]();
}

void dispatch(PingNode* node, std::shared_ptr<yijian::buffer> sp) {

  currentNode_ = node;

  dispatch(sp->datatype(), sp->data(), sp->data_size());

}


#ifdef __cpluscplus
}
#endif

