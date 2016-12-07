#ifndef TYPEMAP_H
#define TYPEMAP_H

#include <stdint.h>
#include "protofiles/chat_message.pb.h"

#ifdef __cpluscplus
extern "C" {
#endif
enum ChatType : uint8_t {

  error,
  registor,
  registorres,
  login,
  loginres,
  clientconnect,
  clientconnectres,
  clientdisconnect,
  clientdisconnectres,
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
  querymessage,
  serverconnect,// outer
  serverdisconnect,// outer
  unread,

};



constexpr uint8_t dispatchType(chat::Error &) {
  return ChatType::error;
}
constexpr uint8_t dispatchType(chat::Register &) {
  return ChatType::registor;
}
constexpr uint8_t dispatchType(chat::RegisterRes &) {
  return ChatType::registorres;
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
constexpr uint8_t dispatchType(chat::QueryMessage & ) {
  return ChatType::querymessage;
}
constexpr uint8_t dispatchType(chat::Unread & ) {
  return ChatType::unread;
}


#ifdef __cpluscplus
}
#endif
#endif
