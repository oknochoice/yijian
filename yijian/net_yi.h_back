#ifndef NET_YI_H
#define NET_YI_H

#include "macro.h"
#include <leveldb/db.h>
#include <string>
#include "lib_client.h"
#include <functional>
#include <map>
#include <mutex>

class netyi {
public:
  // call will stop if isStop not set false
  typedef std::function<void(const std::string&, uint8_t type, bool * isStop)> 
    CB_Func_Mutiple;

  netyi(std::string & dbpath);
  ~netyi();
  /*
   * keys
   * */ 
  // u_$userid
  std::string user_key(const std::string & userid);
  // p_$code_$phoneno
  std::string user_phone_key(const std::string & countrycode,
      const std::string phoneno);
  // n_$tonodeid
  std::string node_key(const std::string & tonodeid);
  // m_$tonodeid_$incrementid
  std::string msg_key(const std::string & tonodeid,
      const std::string & incrementid);
  std::string msg_key(const std::string & tonodeid,
      const int32_t incrementid);
  // t_&userid
  std::string talklist_key(const std::string & userid);
  // e_userid_$nth [1, current]
  std::string error_key(const int32_t nth);
  int32_t get_errorno();
  int32_t get_next_errorno_add();
  // user busness
  std::string signup_login_connect_key();
  std::string logout_disconnect_key();
  std::string unread_noti_key();
  std::string userinfo_noti_key();
  // e_userid_sessionid_$nth [1, maxth]
  std::string session_key(const int32_t sessionid, 
      const int32_t nth);
  int32_t get_sessionid_no(const int32_t sessionid);
  /*
   * common leveldb put get
   * */
  leveldb::Status put(const leveldb::Slice & key, 
      const leveldb::Slice & value);
  leveldb::Status get(const leveldb::Slice & key, 
      std::string & value);
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

  /*
   * current
   * */
  void set_current_userid(const std::string & userid);
  std::string get_current_userid();
  
private:
  int32_t get_next_sessionid_no_add(const int32_t sessionid);
  void    clean_sessionid_keys(const int32_t sessionid);
  void put_map(const int32_t sessionid, CB_Func_Mutiple && func);
  void put_map_send(Buffer_SP sp, CB_Func_Mutiple && func);
  void put_map_send(const int32_t sessionid, 
      Buffer_SP sp, CB_Func_Mutiple && func);
  bool call_map(const int32_t sessionid, Buffer_SP sp);
  bool call_map(const int32_t sessionid, std::string & key, Buffer_SP sp);
private:
  leveldb::DB * db_;
  std::string current_userid = "unknow";
  std::mutex sessionid_map_mutex_;
  std::map<int32_t, CB_Func_Mutiple> sessionid_cbfunc_map_;
};

#endif
