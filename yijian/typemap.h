#ifndef TYPEMAP_H
#define TYPEMAP_H

#include <stdint.h>
#include "protofiles/chat_message.pb.h"
#include <unordered_map>

#ifdef __cpluscplus
extern "C" {
#endif
enum ChatType : uint8_t {

  error,
  registor,
  registorres,
  login,
  loginres,
  loginnoti,
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
  querynodeversion,
  querynodeversionres,
  addfriend,
  addfriendres,
  addfriendnoti,
  addfriendauthorize,
  addfriendauthorizeres,
  addfriendauthorizenoti,
  creategroup,
  creategroupres,
  groupaddmember,
  groupaddmemberres,
  nodemessage,
  nodemessageres,
  nodemessagenoti,
  querymessage,
  queryonemessage,
  media,
  mediares,
  querymedia,
  mediaisexist,
  mediaisexistres,
  mediacheck,
  mediacheckres,
  serverconnect,// outer
  serverdisconnect,// outer
};

std::unordered_map<int32_t, bool>
isMustCheckSessionid();


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
constexpr uint8_t dispatchType(chat::LoginRes &) {
  return ChatType::loginres;
}
constexpr uint8_t dispatchType(chat::LoginNoti & ) {
  return ChatType::loginnoti;
}
constexpr uint8_t dispatchType(chat::Logout & ) {
  return ChatType::logout;
}
constexpr uint8_t dispatchType(chat::LogoutRes & ) {
  return ChatType::logoutres;
}
constexpr uint8_t dispatchType(chat::ClientConnect & ) {
  return ChatType::clientconnect;
}
constexpr uint8_t dispatchType(chat::ClientConnectRes & ) {
  return ChatType::clientconnectres;
}
constexpr uint8_t dispatchType(chat::ClientDisConnect & ) {
  return ChatType::clientdisconnect;
}
constexpr uint8_t dispatchType(chat::ClientDisConnectRes & ) {
  return ChatType::clientdisconnectres;
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
constexpr uint8_t dispatchType(chat::QueryNodeVersion & ) {
  return ChatType::querynodeversion;
}
constexpr uint8_t dispatchType(chat::QueryNodeVersionRes & ) {
  return ChatType::querynodeversionres;
}
constexpr uint8_t dispatchType(chat::AddFriend & ) {
  return ChatType::addfriend;
}
constexpr uint8_t dispatchType(chat::AddFriendRes & ) {
  return ChatType::addfriendres;
}
constexpr uint8_t dispatchType(chat::AddFriendNoti & ) {
  return ChatType::addfriendnoti;
}
constexpr uint8_t dispatchType(chat::AddFriendAuthorize & ) {
  return ChatType::addfriendauthorize;
}
constexpr uint8_t dispatchType(chat::AddFriendAuthorizeRes & ) {
  return ChatType::addfriendres;
}
constexpr uint8_t dispatchType(chat::AddFriendAuthorizeNoti & ) {
  return ChatType::addfriendauthorizenoti;
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
constexpr uint8_t dispatchType(chat::NodeMessageNoti & ) {
  return ChatType::nodemessagenoti;
}
constexpr uint8_t dispatchType(chat::QueryMessage & ) {
  return ChatType::querymessage;
}
constexpr uint8_t dispatchType(chat::QueryOneMessage & ) {
  return ChatType::queryonemessage;
}
constexpr uint8_t dispatchType(chat::Media & ) {
  return ChatType::media;
}
constexpr uint8_t dispatchType(chat::MediaRes & ) {
  return ChatType::mediares;
}
constexpr uint8_t dispatchType(chat::QueryMedia & ) {
  return ChatType::querymedia;
}
constexpr uint8_t dispatchType(chat::MediaIsExist & ) {
  return ChatType::mediaisexist;
}
constexpr uint8_t dispatchType(chat::MediaIsExistRes & ) {
  return ChatType::mediaisexistres;
}
constexpr uint8_t dispatchType(chat::MediaCheck & ) {
  return ChatType::mediacheck;
}
constexpr uint8_t dispatchType(chat::MediaCheckRes & ) {
  return ChatType::mediacheckres;
}


#ifdef __cpluscplus
}
#endif
#endif
