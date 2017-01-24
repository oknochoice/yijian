#ifndef NET_YI_H
#define NET_YI_H

#include "macro.h"
#include <string>
#include "lib_client.h"
#include <functional>
#include <map>
#include <mutex>

class netyi {
public:
  // call will stop if isStop not set false
  typedef std::function<void(uint8_t type, const char * header,
      const int32_t length, bool * isStop)> 
    CB_Func_Mutiple;

  netyi();
  ~netyi();
  /*
   * leveldb keys suggest
   * */ 
  // u_$userid
  // up_$code_$phoneno
  // n_$tonodeid
  // m_$tonodeid_$incrementid
  // t_&userid
  // e_userid_$nth 
  /*
   * net user
   * */
  void net_connect();
  // signup login connect
  // func(key, bool)
  void signup_login_connect(Buffer_SP sp, CB_Func_Mutiple && func);
  // func(key, bool)
  void logout_disconnect(Buffer_SP sp, CB_Func_Mutiple && func);
  // func(sessionid, bool)
  void send_buffer(Buffer_SP sp, CB_Func_Mutiple && func);
  /*
   * net noti
   *
   * */
  // unread msg noti func(key)
  void acceptUnreadMsg(CB_Func_Mutiple && func);
  // other device loginNoti addFriendNoti addFriendAuthorizeNoti
  // func (key)
  void userInfoNoti(CB_Func_Mutiple && func);

private:
  void put_map(const int32_t sessionid, CB_Func_Mutiple && func);
  void put_map_send(Buffer_SP sp, CB_Func_Mutiple && func);
  void put_map_send(const int32_t sessionid, 
      Buffer_SP sp, CB_Func_Mutiple && func);
  bool call_map(const int32_t sessionid, Buffer_SP sp);
private:
  std::mutex sessionid_map_mutex_;
  std::map<int32_t, CB_Func_Mutiple> sessionid_cbfunc_map_;
};

#endif
