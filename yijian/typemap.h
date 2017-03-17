#ifndef TYPEMAP_H
#define TYPEMAP_H
#include <stdint.h>

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
  querymessageres = 34,
  queryonemessage = 35,
  media = 36,
  mediares = 37,
  querymedia = 38,
  querymediares = 39,
  serverconnect = 42,// outer
  serverdisconnect = 43,// outer
  queryaddfriendinfo = 44,
  queryaddfriendinfores = 45,
  ping = 46,
  pong = 47,
  setuserproterty = 48,
  setuserprotertyres = 49
};

bool missing_check(int32_t type);

#endif
