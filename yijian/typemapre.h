#ifndef TYPEMAPRE_H
#define TYPEMAPRE_H

#include "protofiles/chat_message.pb.h"
#include "typemap.h"

#ifdef __cpluscplus
extern "C" {
#endif

constexpr uint8_t dispatchType(const chat::Error &) {
  return ChatType::error;
}
constexpr uint8_t dispatchType(const chat::Register &) {
  return ChatType::registor;
}
constexpr uint8_t dispatchType(const chat::RegisterRes &) {
  return ChatType::registorres;
}
constexpr uint8_t dispatchType(const chat::Login &) {
  return ChatType::login;
}
constexpr uint8_t dispatchType(const chat::LoginRes &) {
  return ChatType::loginres;
}
constexpr uint8_t dispatchType(const chat::LoginNoti & ) {
  return ChatType::loginnoti;
}
constexpr uint8_t dispatchType(const chat::Logout & ) {
  return ChatType::logout;
}
constexpr uint8_t dispatchType(const chat::LogoutRes & ) {
  return ChatType::logoutres;
}
constexpr uint8_t dispatchType(const chat::ClientConnect & ) {
  return ChatType::clientconnect;
}
constexpr uint8_t dispatchType(const chat::ClientConnectRes & ) {
  return ChatType::clientconnectres;
}
constexpr uint8_t dispatchType(const chat::ClientDisConnect & ) {
  return ChatType::clientdisconnect;
}
constexpr uint8_t dispatchType(const chat::ClientDisConnectRes & ) {
  return ChatType::clientdisconnectres;
}
constexpr uint8_t dispatchType(const chat::QueryUser & ) {
  return ChatType::queryuser;
}
constexpr uint8_t dispatchType(const chat::QueryUserRes & ) {
  return ChatType::queryuserres;
}
constexpr uint8_t dispatchType(const chat::QueryUserVersion & ) {
  return ChatType::queryuserversion;
}
constexpr uint8_t dispatchType(const chat::QueryUserVersionRes & ) {
  return ChatType::queryuserversionres;
}
constexpr uint8_t dispatchType(const chat::QueryNode & ) {
  return ChatType::querynode;
}
constexpr uint8_t dispatchType(const chat::QueryNodeRes & ) {
  return ChatType::querynoderes;
}
constexpr uint8_t dispatchType(const chat::QueryNodeVersion & ) {
  return ChatType::querynodeversion;
}
constexpr uint8_t dispatchType(const chat::QueryNodeVersionRes & ) {
  return ChatType::querynodeversionres;
}
constexpr uint8_t dispatchType(const chat::AddFriend & ) {
  return ChatType::addfriend;
}
constexpr uint8_t dispatchType(const chat::AddFriendRes & ) {
  return ChatType::addfriendres;
}
constexpr uint8_t dispatchType(const chat::AddFriendNoti & ) {
  return ChatType::addfriendnoti;
}
constexpr uint8_t dispatchType(const chat::AddFriendAuthorize & ) {
  return ChatType::addfriendauthorize;
}
constexpr uint8_t dispatchType(const chat::AddFriendAuthorizeRes & ) {
  return ChatType::addfriendauthorizeres;
}
constexpr uint8_t dispatchType(const chat::AddFriendAuthorizeNoti & ) {
  return ChatType::addfriendauthorizenoti;
}
constexpr uint8_t dispatchType(const chat::CreateGroup & ) {
  return ChatType::creategroup;
}
constexpr uint8_t dispatchType(const chat::CreateGroupRes & ) {
  return ChatType::creategroupres;
}
constexpr uint8_t dispatchType(const chat::GroupAddMember & ) {
  return ChatType::groupaddmember;
}
constexpr uint8_t dispatchType(const chat::GroupAddMemberRes & ) {
  return ChatType::groupaddmemberres;
}
constexpr uint8_t dispatchType(const chat::NodeMessage & ) {
  return ChatType::nodemessage;
}
constexpr uint8_t dispatchType(const chat::NodeMessageRes & ) {
  return ChatType::nodemessageres;
}
constexpr uint8_t dispatchType(const chat::NodeMessageNoti & ) {
  return ChatType::nodemessagenoti;
}
constexpr uint8_t dispatchType(const chat::QueryMessage & ) {
  return ChatType::querymessage;
}
constexpr uint8_t dispatchType(const chat::QueryOneMessage & ) {
  return ChatType::queryonemessage;
}
constexpr uint8_t dispatchType(const chat::Media & ) {
  return ChatType::media;
}
constexpr uint8_t dispatchType(const chat::MediaRes & ) {
  return ChatType::mediares;
}
constexpr uint8_t dispatchType(const chat::QueryMedia & ) {
  return ChatType::querymedia;
}
constexpr uint8_t dispatchType(const chat::QueryMediaRes & ) {
  return ChatType::querymediares;
}
constexpr uint8_t dispatchType(const chat::QueryAddfriendInfo & ) {
  return ChatType::queryaddfriendinfo;
}
constexpr uint8_t dispatchType(const chat::QueryAddfriendInfoRes & ) {
  return ChatType::queryaddfriendinfores;
}
constexpr uint8_t dispatchType(const chat::Ping & ) {
  return ChatType::ping;
}
constexpr uint8_t dispatchType(const chat::Pong & ) {
  return ChatType::pong;
}
constexpr uint8_t dispatchType(const chat::SetUserProperty & ) {
  return ChatType::setuserproterty;
}
constexpr uint8_t dispatchType(const chat::SetUserPropertyRes & ) {
  return ChatType::setuserprotertyres;
}

#ifdef __cpluscplus
}
#endif
#endif
