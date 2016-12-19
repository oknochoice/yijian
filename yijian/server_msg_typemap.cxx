#include "server_msg_typemap.h"

#include <map>
#include <mutex>

#include "mongo.h"
#include "protofiles/chat_message.pb.h"
#include <string>
#include <queue>
#include "buffer.h"
#include "libev_server.h"
#include <unordered_map>


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
    Buffer_SP errorBuffer(uint_fast32_t err_num, std::string && err_msg) {
      auto error = chat::Error();
      error.set_errnum(err_num);
      error.set_errmsg(err_msg);
      return buffer::Buffer(error);
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

// user require device
void mountBuffer2Node(Buffer_SP buf_sp, chat::NodeSelfDevice & ) {
  YILOG_TRACE ("func: {}. self device", __func__);
  buf_sp->set_sessionid(currentNode_->sessionid - 1);
  std::unique_lock<std::mutex> ul(currentNode_->writeio->buffers_p_mutex);
  currentNode_->writeio->buffers_p.push(buf_sp);
  YILOG_TRACE ("func: {}. self device, write queue count {}", 
      __func__, currentNode_->writeio->buffers_p.size());
  // push node
  yijian::threadCurrent::pushPingnode(currentNode_);
}

// peer server
void mountBuffer2Node(Buffer_SP buf_sp, chat::NodePeerServer & ) {
  YILOG_TRACE ("func: {}. peer server", __func__);
  //transmit to peer server
  auto sp = peer_servers();
  for (auto & lnode: *sp) {
    // mount buffer to pingnode
    {
      std::unique_lock<std::mutex> ul(lnode->writeio->buffers_p_mutex);
      lnode->writeio->buffers_p.push(buf_sp);
      YILOG_TRACE ("func: {}. peer server, write queue count {}", 
          __func__, lnode->writeio->buffers_p.size());
      // push node
      yijian::threadCurrent::pushPingnode(lnode);
    }
    // mount pingnode to thread data ,then stop read start write
    yijian::threadCurrent::pushPingnode(lnode);
  }
}

void traverseDevices(chat::ConnectInfoLittle & infolittle, Buffer_SP buf_sp) {
  if (infolittle.isconnected()) {
    YILOG_TRACE ("online");
    // get pingnode
    Pointor_t nodepointor = infolittle.nodepointor();
    Read_IO * lnode = reinterpret_cast<Read_IO*>(nodepointor);
    // if node is request pass
    if (unlikely(currentNode_ == lnode)) return;
    // mount buffer to pingnode
    {
      std::unique_lock<std::mutex> ul(lnode->writeio->buffers_p_mutex);
      lnode->writeio->buffers_p.push(buf_sp);
      YILOG_TRACE ("func: {}. , write queue count {}", 
          __func__, lnode->writeio->buffers_p.size());
    }
    // mount pingnode to thread data ,then stop read start write
    yijian::threadCurrent::pushPingnode(lnode);
  }
}

void traverseDevices(chat::ConnectInfoLittle & infolittle,
    const std::string & tonodeid,
    const int32_t incrementid) {
  if (infolittle.isconnected()) {
    YILOG_TRACE ("online");
    // get pingnode
    Pointor_t nodepointor = infolittle.nodepointor();
    Read_IO * lnode = reinterpret_cast<Read_IO*>(nodepointor);
    // if node is request pass
    if (unlikely(currentNode_ == lnode)) return;
    // mount buffer to pingnode
    {

      auto unread = (*lnode->unread_sp).mutable_unreadnodes();
      (*unread)[tonodeid] = incrementid;
      auto buf_sp = buffer::Buffer(*lnode->unread_sp);

      std::unique_lock<std::mutex> ul(lnode->writeio->buffers_p_mutex);
      lnode->writeio->buffers_p.push(buf_sp);
      YILOG_TRACE ("func: {}. , write queue count {}", 
          __func__, lnode->writeio->buffers_p.size());
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


// current server subscribe to toNode devices(exclude require device)
void mountBuffer2Node(Buffer_SP buf_sp, chat::NodeSpecifiy & node_specifiy) {
  YILOG_TRACE ("func: {}. node specifiy", __func__);
  auto inClient = yijian::threadCurrent::inmemClient();
  auto client = yijian::threadCurrent::mongoClient();
  auto node_sp = client->queryNode(node_specifiy.tonodeid());
  inClient->devices(node_sp->members(), 
      [buf_sp](chat::ConnectInfoLittle & infolittle) {
        traverseDevices(infolittle, buf_sp);
      });
}

void mountBuffer2Node(Buffer_SP buf_sp, chat::NodeUser & node_user) {
  YILOG_TRACE ("func: {}. node user", __func__);
  auto inClient = yijian::threadCurrent::inmemClient();
  inClient->devices(node_user, 
      [buf_sp](chat::ConnectInfoLittle & infolittle) {
        traverseDevices(infolittle, buf_sp);
      });
}

void mountUnread2Node(const std::string & tonodeid, 
    const int32_t incrementid, 
    chat::NodeSpecifiy & node_specifiy) {
  YILOG_TRACE ("func: {}. node user", __func__);
  auto inClient = yijian::threadCurrent::inmemClient();
  auto client = yijian::threadCurrent::mongoClient();
  auto node_sp = client->queryNode(node_specifiy.tonodeid());
  inClient->devices(node_sp->members(), 
      [&tonodeid, incrementid](chat::ConnectInfoLittle & infolittle) {
        traverseDevices(infolittle, tonodeid, incrementid);
      });
}

void mountUnread2Node(const std::string & tonodeid, 
    const int32_t incrementid, 
    chat::NodeUser & node_user) {
  YILOG_TRACE ("func: {}. node user", __func__);
  auto inClient = yijian::threadCurrent::inmemClient();
  inClient->devices(node_user, 
      [&tonodeid, incrementid](chat::ConnectInfoLittle & infolittle) {
        traverseDevices(infolittle, tonodeid, incrementid);
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


  try {

    auto client = yijian::threadCurrent::mongoClient();
    auto id = client->insertUser(enroll);

    YILOG_TRACE ("func: {}. register, userid {}.", 
        __func__, id);

    auto res = chat::RegisterRes();
    res.set_userid(id);
    res.set_issuccess(true);
    mountBuffer2Node(buffer::Buffer(res), node_self_);

  }catch (std::system_error & sys_error) {
    auto res = chat::RegisterRes();
    res.set_issuccess(false);
    res.set_e_no(sys_error.code().value());
    res.set_e_msg(sys_error.what());
    mountBuffer2Node(buffer::Buffer(res), node_self_);
  }

}

void dispatch(chat::Login & login) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto client = yijian::threadCurrent::mongoClient();
  auto inmem_client = yijian::threadCurrent::inmemClient();

  try {
    auto user_sp = client->queryUser(login.phoneno(), login.countrycode());
    if (user_sp->password() == login.password()) {
      // setup device
      auto device = std::make_shared<chat::Device>();
      device->set_os(login.device().os());
      device->set_devicemodel(login.device().devicemodel());
      device->set_uuid(login.device().uuid());
      // find device from db
      auto devices = user_sp->mutable_devices();
      auto it = find_if(devices->begin(), devices->end(),
                  [&](const chat::Device& device) -> bool {
                    return device.uuid() == login.device().uuid();
                  });
      // check whether finded
      if (it != devices->end()) {
        // update user
        client->updateDevice(user_sp->id(), *device);
      }else {
        client->insertDevice(user_sp->id(), *device);
      }
      // add global user connect info
      auto isFind = inmem_client->findUUID(device->uuid(), connectInfo_);
      if (isFind) {
        auto users_inconnectinfo = connectInfo_.mutable_users();
        auto it = users_inconnectinfo->find(user_sp->id());
        if (it == users_inconnectinfo->end()) {// not find
          (*users_inconnectinfo)[user_sp->id()] = 1;
        }else {
        }
        connectInfo_.set_uuid(device->uuid());
        connectInfo_.set_userid(user_sp->id());
        connectInfo_.set_islogin(true);
        connectInfo_.set_isconnected(false);
        connectInfo_.set_isrecivenoti(false);
        connectInfo_.set_servername(SERVER_NAME);
        connectInfo_.set_nodepointor(0);

        inmem_client->updateUUID(connectInfo_);
      }else {
        connectInfo_.set_uuid(device->uuid());
        connectInfo_.set_userid(user_sp->id());
        connectInfo_.set_islogin(true);
        connectInfo_.set_isconnected(false);
        connectInfo_.set_isrecivenoti(false);
        connectInfo_.set_servername(SERVER_NAME);
        connectInfo_.set_nodepointor(0);
        connectInfo_.clear_users();
        auto users_inconnectinfo = connectInfo_.mutable_users();
        (*users_inconnectinfo)[user_sp->id()] = 1;

        // insert connect info
        inmem_client->insertUUID(connectInfo_);
      }
      // set current node
      currentNode_->userid = user_sp->id();
      currentNode_->deviceid = login.device().uuid();
      currentNode_->sessionid = connectInfo_.users().at(user_sp->id());
      // response request device
      auto res = chat::LoginRes();
      res.set_issuccess(true);
      res.set_uuid(device->uuid());
      mountBuffer2Node(buffer::Buffer(res), node_self_);
      res.set_userid(currentNode_->userid);
      mountBuffer2Node(buffer::Buffer(res), node_peer_);
      node_user_.set_touserid(currentNode_->userid);
      mountBuffer2Node(buffer::Buffer(res), node_user_);
    }else {
      mountBuffer2Node(errorBuffer(11001, "password or account error"),
          node_self_);
    }

  }catch (std::system_error & sys_error) {
    auto res = chat::LoginRes();
    res.set_issuccess(false);
    res.set_e_no(sys_error.code().value());
    res.set_e_msg(sys_error.what());
    mountBuffer2Node(buffer::Buffer(res), node_self_);
  }

}

void dispatch(chat::LoginRes & res) {

  YILOG_TRACE ("func: {}. ", __func__);
  try {
    node_user_.set_touserid(res.userid());
    res.clear_userid();
    mountBuffer2Node(buffer::Buffer(res), node_user_);
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
      // update connect info
      auto inmem_client = yijian::threadCurrent::inmemClient();
      auto isFind = inmem_client->findUUID(it->uuid(), connectInfo_);
      if (unlikely(!isFind)) {
        YILOG_ERROR ("logout not find connection info");
      }
      connectInfo_.set_isconnected(false);
      connectInfo_.set_islogin(false);
      connectInfo_.set_nodepointor(0);
      auto users_inconnectinfo = connectInfo_.mutable_users();
      (*users_inconnectinfo)[logout.uuid()] = currentNode_->sessionid;
      inmem_client->updateUUID(connectInfo_);

      currentNode_->isConnect = false;
      // send buffer
      auto res = chat::LogoutRes();
      res.set_uuid(it->uuid());
      mountBuffer2Node(buffer::Buffer(res), node_self_);
      node_user_.set_touserid(currentNode_->userid);
      res.set_userid(currentNode_->userid);
      mountBuffer2Node(buffer::Buffer(res), node_user_);
      mountBuffer2Node(buffer::Buffer(res), node_peer_);
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
    mountBuffer2Node(buffer::Buffer(res), node_user_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }
}
void dispatch(chat::ClientConnect & connect)  {

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
      auto inmem_client = yijian::threadCurrent::inmemClient();
      auto isFind = inmem_client->findUUID(connect.uuid(), connectInfo_);
      if (unlikely(!isFind)) {
        YILOG_ERROR ("connect not find connection info");
      }
      // set current node
      currentNode_->userid = connect.userid();
      currentNode_->deviceid = connect.uuid();
      currentNode_->sessionid = connectInfo_.users().at(connect.userid());
      currentNode_->isConnect = true;
      currentNode_->clientVersion = connect.clientversion();
      currentNode_->appVersion = connect.appversion();
      // update in memory db connectinfo
      connectInfo_.set_isconnected(true);
      connectInfo_.set_isrecivenoti(connect.isrecivenoti());
      connectInfo_.set_nodepointor(reinterpret_cast<Pointor_t>(currentNode_));
      connectInfo_.set_clientversion(connect.clientversion());
      connectInfo_.set_appversion(connect.appversion());
      connectInfo_.set_osversion(connect.osversion());
      inmem_client->updateUUID(connectInfo_);
      // send buffer
      auto res = chat::ClientConnectRes();
      res.set_issuccess(true);
      res.set_uuid(connectInfo_.uuid());
      res.set_sessionid(currentNode_->sessionid);
      mountBuffer2Node(buffer::Buffer(res), node_self_);
    }
  }catch (std::system_error & sys_error) {
    auto res = chat::ClientConnectRes();
    res.set_issuccess(false);
    res.set_e_no(sys_error.code().value());
    res.set_e_msg(sys_error.what());
    mountBuffer2Node(buffer::Buffer(res), node_self_);
  }

}

void dispatch(chat::ClientDisConnect & disconnect)  {

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
      auto isFind = inmem_client->findUUID(disconnect.uuid(), connectInfo_);
      if (unlikely(!isFind)) {
        YILOG_ERROR ("disconnect not find connection info");
      }
      connectInfo_.set_isconnected(false);
      connectInfo_.set_nodepointor(0);
      auto users_inconnectinfo = connectInfo_.mutable_users();
      (*users_inconnectinfo)[disconnect.userid()] = currentNode_->sessionid;
      inmem_client->updateUUID(connectInfo_);
      // send buffer
      auto res = chat::ClientDisConnectRes();
      res.set_uuid(connectInfo_.uuid());
      mountBuffer2Node(buffer::Buffer(res), node_self_);
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
  /*
  */
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
    // check whether is request already (db maybe insert failure(unlikely))
    auto user_sp = client->queryUser(frd.inviterid());
    auto & user_reqs = user_sp->friendreqs();
    auto user_it = find_if(user_reqs.begin(), user_reqs.end(),
        [&](const chat::FriendRequest & req) -> bool {
          return req.userid() == frd.inviteeid();
        });
    if (unlikely(user_it != user_reqs.end())) {
      if (user_it->isinviter()) {
        throw std::system_error(std::error_code(11020, 
              std::generic_category()),
              "as inviter add friend already");
      }else {
        throw std::system_error(std::error_code(11021, 
              std::generic_category()),
              "as invitee add friend already");
      }
    }
    // double check is request already (db insert failure)
    auto & peer_reqs = peerUser->friendreqs();
    auto peer_it = find_if(peer_reqs.begin(), peer_reqs.end(),
        [&](const chat::FriendRequest & req) -> bool {
          return req.userid() == frd.inviterid();
        });

    if (unlikely(peer_it != peer_reqs.end())) {
      if (peer_it->isinviter()) {
        throw std::system_error(std::error_code(11022, 
              std::generic_category()),
              "as inviter add friend already and db insert failure");
      }else {
        throw std::system_error(std::error_code(11023, 
              std::generic_category()),
              "as invitee add friend already and db insert failure");
      }
    }
    // insert inviter invitee
    auto res = std::make_shared<chat::AddFriendRes>();
    // send buffer
    auto buf_sp = buffer::Buffer(*res);
    mountBuffer2Node(buf_sp, node_self_);

    // send to friend
    node_user_.set_touserid(res->inviteeid());
    mountBuffer2Node(buf_sp, node_user_);
    mountBuffer2Node(buffer::Buffer(*res), node_peer_);
    // send to other self
    node_user_.set_touserid(res->inviterid());
    mountBuffer2Node(buf_sp, node_user_);
    mountBuffer2Node(buffer::Buffer(*res), node_peer_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::AddFriendRes & frdRes) {

  YILOG_TRACE ("func: {}. ", __func__);

  try {
    auto buf_sp = buffer::Buffer(frdRes);
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
  auto client = yijian::threadCurrent::mongoClient();
  try {
    if (unlikely(addAuth.inviteeid() != currentNode_->userid)) {
      throw std::system_error(std::error_code(11024, std::generic_category()),
            "no request in inviter");
    }
    auto user_sp = client->queryUser(addAuth.inviteeid());
    auto & user_reqs = user_sp->friendreqs();
    auto user_it = find_if(user_reqs.begin(), user_reqs.end(),
        [&](const chat::FriendRequest & req) -> bool {
          return req.userid() == addAuth.inviterid() &&
                  req.isinviter() == false;
        });
    if (unlikely(user_it == user_reqs.end())) {
      throw std::system_error(std::error_code(11025, std::generic_category()),
            "no request in inviter");
    }
    auto peer_sp = client->queryUser(addAuth.inviterid());
    auto & peer_reqs = peer_sp->friendreqs();
    auto peer_it = find_if(peer_reqs.begin(), peer_reqs.end(),
        [&](const chat::FriendRequest & req) -> bool {
          return req.userid() == addAuth.inviteeid() &&
                  req.isinviter() == true;
        });
    if (unlikely(peer_it == peer_reqs.end())) {
      throw std::system_error(std::error_code(11026, std::generic_category()),
            "no request in invitee");
    }
    if (addAuth.isagree() == true) {
      // add friend
      client->addFriendAuthorize(addAuth.inviterid(),
          addAuth.inviteeid(), addAuth.tonodeid());
      // send 
      auto authRes = chat::AddFriendAuthorizeRes();
      authRes.set_isagree(chat::IsAgree::agree);
      // inviter
      authRes.set_touserid_outer(addAuth.inviterid());
      node_user_.set_touserid(addAuth.inviterid());
      auto inviter_buf = buffer::Buffer(authRes);
      mountBuffer2Node(inviter_buf, node_user_);
      mountBuffer2Node(inviter_buf, node_peer_);
      // invitee
      authRes.set_touserid_outer(addAuth.inviteeid());
      node_user_.set_touserid(addAuth.inviteeid());
      auto invitee_buf = buffer::Buffer(authRes);
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
    node_user_.set_touserid(authRes.touserid_outer());
    mountBuffer2Node(buffer::Buffer(authRes), node_user_);
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
    // send to self
    mountBuffer2Node(buffer::Buffer(*groupRes), node_self_);
    // self other device
    node_user_.set_touserid(currentNode_->userid);
    mountBuffer2Node(buffer::Buffer(*groupRes), node_user_);
    // peer 
    groupRes->set_touserid_outer(currentNode_->userid);
    mountBuffer2Node(buffer::Buffer(*groupRes), node_peer_);
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
    mountBuffer2Node(buffer::Buffer(groupRes), node_user_);
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
    // send to self
    mountBuffer2Node(buffer::Buffer(*addRes), node_self_);
    // send to self other device
    node_user_.set_touserid(addRes->touserid_outer());
    mountBuffer2Node(buffer::Buffer(*addRes), node_user_);
    // send to peer
    addRes->set_touserid_outer(currentNode_->userid);
    mountBuffer2Node(buffer::Buffer(*addRes), node_peer_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::GroupAddMemberRes & addMemberRes) {

  YILOG_TRACE ("func: {}. ", __func__);
  try {
    node_user_.set_touserid(addMemberRes.touserid_outer());
    mountBuffer2Node(buffer::Buffer(addMemberRes), node_user_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()),
        node_self_);
  }

}

void dispatch(chat::QueryUserVersion & queryVersion) {
  YILOG_TRACE ("func: {}. ", __func__);
  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto user_sp = client->queryUser(queryVersion.userid());
    auto version = chat::QueryUserVersionRes();
    version.set_version(user_sp->version());
    version.set_userid(queryVersion.userid());
    mountBuffer2Node(buffer::Buffer(version), node_self_);
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
      user_sp->clear_friendreqs();
    }
    *queryuser = *user_sp;
    mountBuffer2Node(buffer::Buffer(queryUserRes), node_self_);
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
    mountBuffer2Node(buffer::Buffer(querynoderes), node_self_);
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
      auto encoding_res_sp = buffer::Buffer(*res);
      mountBuffer2Node(encoding_res_sp, node_self_);
      mountBuffer2Node(encoding_res_sp, node_peer_);

      // send to node
      node_specifiy_.set_tonodeid(res->tonodeid());
      mountBuffer2Node(buffer::Buffer(message), node_specifiy_);
    }else {// user
      // self
      mountBuffer2Node(buffer::Buffer(*res), node_self_);

      // to friend
      node_user_.set_touserid(message.touserid_outer());
      mountBuffer2Node(buffer::Buffer(message), node_user_);
      res->set_touserid_outer(message.touserid_outer());
      mountBuffer2Node(buffer::Buffer(*res), node_peer_);
      // to self other device
      node_user_.set_touserid(currentNode_->userid);
      mountBuffer2Node(buffer::Buffer(message), node_user_);
      res->set_touserid_outer(currentNode_->userid);
      mountBuffer2Node(buffer::Buffer(*res), node_peer_);
    }
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::NodeMessageRes & messageRes) {

  YILOG_TRACE ("func: {}. ", __func__);

  try {
    
    if (messageRes.touserid_outer().empty()) {
      // sent to node
      node_specifiy_.set_tonodeid(messageRes.tonodeid());
      mountUnread2Node(messageRes.tonodeid(), messageRes.incrementid(),
          node_specifiy_);
    }else {
      // to user
      node_user_.set_touserid(messageRes.touserid_outer());
      mountUnread2Node(messageRes.tonodeid(), messageRes.incrementid(),
          node_user_);
    }
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }

}

void dispatch(chat::QueryMessage & query) {
  YILOG_TRACE ("func: {}. ", __func__);
  try {
    auto client = yijian::threadCurrent::mongoClient();
    if (query.isreset()) {
      // reset currentnode cursor
      if (likely(nullptr != currentNode_->id_cursor.second)){
          // close cursor
      }
      currentNode_->id_cursor.first = query.tonodeid();
      currentNode_->id_cursor.second = client->cursor(query);
    }
    if (unlikely(currentNode_->id_cursor.first.empty() ||
          currentNode_->id_cursor.second == nullptr)) {
      throw std::system_error(std::error_code(11008, std::generic_category()),
          "query tonodeid is empty");
    }
    auto nodemessage_sp = client->queryMessage(
          currentNode_->id_cursor.second);
    mountBuffer2Node(buffer::Buffer(*nodemessage_sp), node_self_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}
void dispatch(chat::QueryOneMessage & query) {
  YILOG_TRACE ("func: {}. ", __func__);
  try {
    auto client = yijian::threadCurrent::mongoClient();
    auto nodemessage_sp = 
      client->queryMessage(query.tonodeid(), query.incrementid());
    mountBuffer2Node(buffer::Buffer(*nodemessage_sp), node_self_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}

void dispatch(chat::Media & media) {
  YILOG_TRACE ("func: {}. media", __func__);
  try {
    auto mediares = chat::MediaRes();
    mediares.set_sha1(media.sha1());
    mediares.set_nth(media.nth());
    currentNode_->media_vec.push_back(std::move(media));
    mountBuffer2Node(buffer::Buffer(mediares), node_self_);
  }catch (std::system_error & sys_error) {
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}

void dispatch(chat::MediaCheck & mediacheck) {
  YILOG_TRACE ("func: {}. media", __func__);
  try {
    auto client = yijian::threadCurrent::mongoClient();
    client->insertMedia(currentNode_->media_vec);
    currentNode_->media_vec.clear();
    auto mediares = chat::MediaCheckRes();
    mediares.set_sha1(mediacheck.sha1());
    mediares.set_isintact(true);
    mountBuffer2Node(buffer::Buffer(mediares), node_self_);
  }catch (std::system_error & sys_error) {
    currentNode_->media_vec.clear();
    mountBuffer2Node(errorBuffer(sys_error.code().value(), sys_error.what()), 
        node_self_);
  }
}

void dispatch(chat::QueryMedia & querymedia) {
  YILOG_TRACE ("func: {}. media", __func__);
  try {
    auto client = yijian::threadCurrent::mongoClient();
    int32_t maxlength = static_cast<int32_t>(Message_Type::message)
      - PADDING_LENGTH;
    std::vector<std::shared_ptr<chat::Media>> medias;
    client->queryMedia(querymedia.sha1(), medias, maxlength);
    for (auto media_sp: medias) {
      mountBuffer2Node(buffer::Buffer(*media_sp), node_self_);
    }
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
      (*map_p)[ChatType::clientconnect] = [=]() {
        auto chat = chat::ClientConnect();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::clientconnectres] = [=]() {
        auto chat = chat::ClientConnectRes();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::clientdisconnect] = [=]() {
        auto chat = chat::ClientDisConnect();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::clientdisconnectres] = [=]() {
        auto chat = chat::ClientDisConnectRes();
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
  });
  auto it = map_p->find(type);
  if (it != map_p->end()) {
    it->second();
  }else {
    YILOG_ERROR ("map_p not add type: {}.", type);
  }
}

static std::unordered_map<int32_t, bool> 
check_map_ = {
  {ChatType::clientdisconnect, true},
  {ChatType::logout, true},
  {ChatType::queryuser, true},
  {ChatType::queryuserversion, true},
  {ChatType::querynode, true},
  {ChatType::addfriend, true},
  {ChatType::addfriendauthorize, true},
  {ChatType::creategroup, true},
  {ChatType::groupaddmember, true},
  {ChatType::nodemessage, true},
  {ChatType::querymessage, true},
  {ChatType::media, true},
  {ChatType::querymedia, true},
  {ChatType::mediacheck, true},
};


void dispatch(Read_IO* node, std::shared_ptr<yijian::buffer> sp) {

  YILOG_TRACE ("func: {}. argc node sp", __func__);

  currentNode_ = node;

  if (unlikely(currentNode_->sessionid != sp->session_id() && 
              check_map_.find(sp->datatype()) != check_map_.end()
        )) {
    auto error = chat::Error();
    error.set_errnum(11010);
    auto err_msg = "session id error, right id is " + 
      std::to_string(currentNode_->sessionid) + " .";
    error.set_errmsg(err_msg);
    mountBuffer2Node(buffer::Buffer(error), node_peer_);
  }else {
    YILOG_TRACE("func: {}, sp {} {} {}", __func__,
        sp->datatype(), sp->data(), sp->data_size());
    if (unlikely(false == currentNode_->isConnect)) {
      if (likely(sp->datatype() == ChatType::registor ||
              sp->datatype() == ChatType::login ||
              sp->datatype() == ChatType::clientconnect)) {
        ++currentNode_->sessionid;
        dispatch(sp->datatype(), sp->data(), sp->data_size());
      }else {
        auto error = chat::Error();
        error.set_errnum(11011);
        error.set_errmsg("need connect first");
        mountBuffer2Node(buffer::Buffer(error), node_peer_);
      }
    }else {
      ++currentNode_->sessionid;
      dispatch(sp->datatype(), sp->data(), sp->data_size());
    }
  }

}


#ifdef __cpluscplus
}
#endif

