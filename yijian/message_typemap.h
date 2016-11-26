#ifndef MESSAGE_TYPEMAP_H
#define MESSAGE_TYPEMAP_H

#include <typeindex>
#include <boost/hana.hpp>
#include <boost/any.hpp>
#include <functional>
#include "pinglist.h"
#include "protofiles/chat_message.pb.h"
#include "libev_server.h"

#ifdef __cpluscplus
extern "C" {
#endif

namespace yijian {
  namespace threadCurrent {
    static thread_local PingNode * currentNode_;
    static thread_local chat::ConnectInfo connectInfo_;
    static thread_local chat::ConnectInfoLittle infolittle_;
    static thread_local chat::NodeSelfDevice node_self_;
    static thread_local chat::NodePeerServer node_peer_;
    static thread_local chat::NodeUser node_user_;
    static thread_local chat::NodeSpecifiy node_specifiy_;
    Buffer_SP errorBuffer();
    Buffer_SP errorBuffer(uint_fast32_t err_num, std::string && err_msg);
  }
}


enum ChatType : uint8_t {

  error,
  registor,
  login,
  loginres,
  clientconnect,
  clientconnectres,
  disconnect,
  disconnectres,
  logout,
  logoutres,
  queryuser,
  queryuserres,
  queryuserversion,
  queryuserversionres,
  querynode,
  querynoderes,
  addfriend,
  addfriendres,
  addfriendauthorize,
  addfriendauthorizeres,
  creategroup,
  creategroupres,
  groupaddmember,
  groupaddmemberres,
  nodemessage,
  nodemessageres,
  queryonemessage,
  querymessage,
  serverconnect,// outer
  serverdisconnect,// outer

};


void dispatch(int type, char * header, std::size_t length);

void dispatch(PingNode* node, std::shared_ptr<yijian::buffer> sp);


constexpr uint8_t dispatchType(chat::Error &) {
  return ChatType::error;
}
constexpr uint8_t dispatchType(chat::Register &) {
  return ChatType::registor;
}
constexpr uint8_t dispatchType(chat::Login &) {
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
  return ChatType::clientconnect;
}
constexpr uint8_t dispatchType(chat::ConnectRes & ) {
  return ChatType::clientconnectres;
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
constexpr uint8_t dispatchType(chat::QueryNode & ) {
  return ChatType::querynode;
}
constexpr uint8_t dispatchType(chat::QueryNodeRes & ) {
  return ChatType::querynoderes;
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

template <typename Any>
void dispatchType(Any &) {
  YILOG_TRACE ("func: {}. ", __func__);
  throw std::system_error(std::error_code(11009, std::generic_category()), 
      "unkonw dispatch type");
}


template <typename Proto> Buffer_SP
encoding(Proto && any) {

  YILOG_TRACE ("func: {}. ", __func__);

  auto type = dispatchType(any);

  auto buf = std::make_shared<yijian::buffer>();
  buf->encoding(std::forward<Proto>(any), 
      type);
  /*
  buf->data_encoding_length(any.ByteSize());
  buf->data_encoding_type(dispatchType(any));
  any.SerializeToArray(buf->data_encoding_current(), buf->remain_size());
  buf->data_encoding_current_addpos(any.ByteSize());
  */

  return buf;
}

template <typename Any>
void mountBuffer2Node(Any &) {
  YILOG_TRACE ("func: {}. ", __func__);
  throw std::system_error(std::error_code(11007, std::generic_category()), 
      "unkonw node type");
}

template <typename Any>
void dispatch(Any & ) {
  YILOG_TRACE ("func: {}. ", __func__);
  throw std::system_error(std::error_code(11000, std::generic_category()), 
      "unkonw dispatch type");
}

#ifdef __cpluscplus
}
#endif

#endif
