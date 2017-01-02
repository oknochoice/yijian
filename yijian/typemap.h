#ifndef TYPEMAP_H
#define TYPEMAP_H

#include <stdint.h>
#include "protofiles/chat_message.pb.h"
#include <unordered_map>

#ifdef __cpluscplus
extern "C" {
#endif
enum ChatType : uint8_t {

  error = 0,
  registor = 1,
  registorres = 2,
  login = 3,
  loginres = 4,
  loginnoti = 5,
  clientconnect = 6,
  clientconnectres = 7,
  clientdisconnect = 8,
  clientdisconnectres = 9,
  logout = 10,
  logoutres = 11,
  queryuser = 12,
  queryuserres = 13,
  queryuserversion = 14,
  queryuserversionres = 15,
  querynode = 16,
  querynoderes = 17,
  querynodeversion = 18,
  querynodeversionres = 19,
  addfriend = 20,
  addfriendres = 21,
  addfriendnoti = 22,
  addfriendauthorize = 23,
  addfriendauthorizeres = 24,
  addfriendauthorizenoti = 25,
  creategroup = 26,
  creategroupres = 27,
  groupaddmember = 28,
  groupaddmemberres = 29,
  nodemessage = 30,
  nodemessageres = 31,
  nodemessagenoti = 32,
  querymessage = 33,
  queryonemessage = 34,
  media = 35,
  mediares = 36,
  querymedia = 37,
  mediaisexist = 38,
  mediaisexistres = 39,
  mediacheck = 40,
  mediacheckres = 41,
  queryaddfriendinfo = 44,
  queryaddfriendinfores = 45,
  serverconnect = 42,// outer
  serverdisconnect = 43,// outer
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
  return ChatType::addfriendauthorizeres;
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
constexpr uint8_t dispatchType(chat::QueryAddfriendInfo & ) {
  return ChatType::queryaddfriendinfo;
}
constexpr uint8_t dispatchType(chat::QueryAddfriendInfoRes & ) {
  return ChatType::queryaddfriendinfores;
}


#ifdef __cpluscplus
}
#endif
#endif
