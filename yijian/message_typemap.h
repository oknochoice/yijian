#ifndef MESSAGE_TYPEMAP_H
#define MESSAGE_TYPEMAP_H

#include <typeindex>
#include <boost/hana.hpp>
#include <boost/any.hpp>
#include <functional>
#include "pinglist.h"

#ifdef __cpluscplus
extern "C" {
#endif

namespace yijian {
  namespace threadCurrent {
    static thread_local PingNode * currentNode_;
    static thread_local chat::ConnectInfo connectInfo_;
    static thread_local chat::NodeSelfDevice node_self_;
    static thread_local chat::NodePeerServer node_peer_;
    static thread_local chat::NodeUser node_user_;
    static thread_local chat::NodeSpecifiy node_specifiy_;
    Buffer_SP errorBuffer();
    Buffer_SP errorBuffer(uint_fast32_t err_num, std::string && err_msg);
  }
}

template <typename Any>
void dispatch(Any & a);

enum ChatType : uint8_t {

  error,
  registor,
  login,
  connect,
  disconnect,
  logout,
  user,
  userinfo,
  group,
  groupinfo,
  chatmessage,
  serverconnect,
  serverdisconnect,
  notiunread,

};

constexpr uint8_t dispatchType(chat::Error & error);

void dispatch(chat::Error& error);

void dispatch(chat::Register & rollin);

void dispatch(chat::Login & login);

void dispatch(int type, char * header, std::size_t length);

void dispatch(PingNode* node, std::shared_ptr<yijian::buffer> sp);

#ifdef __cpluscplus
}
#endif

#endif
