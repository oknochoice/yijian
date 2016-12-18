#ifndef KV_DB_CLIENT_H
#define KV_DB_CLIENT_H

#include <leveldb/db.h>
#include <string>
#include "macro.h"
#include "lib_client.h"
#include <functional>
#include <map>
#include <mutex>

#define KVDBVersion "1.0.0"

class kvdb {
public:
  // construct...
  kvdb(std::string & path);
  ~kvdb();

  // alias type
  typedef std::function<void(std::string&)> CB_Func;

  // 50000
  
  /*
   * common
   *
   * */
  leveldb::Status put(const leveldb::Slice & key, 
      const leveldb::Slice & value);
  leveldb::Status get(const leveldb::Slice & key, 
      std::string & value);
  // u_$userid
  std::string userKey(const std::string & userid);
  // m_$tonodeid_$incrementid
  std::string msgKey(const std::string & tonodeid,
      const std::string & incrementid);
  // p_$code_$phoneno
  std::string userPhoneKey(const std::string & countrycode,
      const std::string phoneno);
  // e_userid_$nth
  std::string errorKey(const std::string & userid,
      const std::string & nth);
  // t_&userid
  std::string talklistKey(const std::string & userid);

  
  // signup_kvdb
  std::string signupKey();
  // login_kvdb
  std::string loginKey();
  // logout_kvdb
  std::string logoutKey();
  // connect_kvdb
  std::string connectKey();
  // disconnect_kvdb
  std::string disconnectKey();


  /*
   * current 
   *
   * */
  // current user key = current_user
  void set_current_userid(const std::string & userid);
  std::string get_current_userid();
  void set_current_error(Buffer_SP sp);
  int32_t get_current_error_maxth();//[1, maxth)

  /*
   * user
   *
   * */
  leveldb::Status putUser(const std::string & userid, 
      const std::string & countrycode,
      const std::string & phoneno,
      const leveldb::Slice & user);
  leveldb::Status getUser(const std::string & id, 
      std::string & user);
  leveldb::Status getUser(const std::string & countrycode,
      const std::string & phoneno,
      std::string & user);


  /*
   * message
   *
   * */ 
  leveldb::Status putTalklist(const std::string & userid, 
      const leveldb::Slice & talklist);
  leveldb::Status getTalklist(const std::string & userid, 
      std::string & talklist);
  leveldb::Status putMsg(const std::string & msgNode,
      const int32_t & incrementid,
      const leveldb::Slice & msg);
  leveldb::Status getMsg(const std::string & msgNode,
      const int32_t & incrementid,
      std::string & msg);

  // 50010

  /*
   * busness regist login connect
   *
   * */ 
  void registUser(const std::string & phoneno,
                  const std::string & countrycode,
                  const std::string & password,
                  const std::string & verifycode,
                  const std::string & nickname,
                  CB_Func && func);
  // OS, iOS = 0, Android = 1;
  void login(const std::string & phoneno,
             const std::string & countrycode,
             const std::string & password,
             const int os,
             const std::string & devicemodel,
             const std::string & uuid,
             CB_Func && func);
  void logout(const std::string & userid,
              const std::string & uuid,
              CB_Func && func);
  void connect(const std::string & userid,
               const std::string & uuid,
               const bool isrecivenoti,
               const std::string & OSversion,
               const std::string & appversion,
               CB_Func && func);
  void disconnect(const std::string & userid,
               const std::string & uuid,
               CB_Func && func);

  /*
   * busness add friend
   *
   * */
  void addfriend(const std::string & inviteeid,
                 const std::string & msg,
                 CB_Func && func);
  // agree = 0, refuse = 1, ignore = 2,
  void addfriendAuthorize(const std::string & inviterid,
                          const std::string & tonodeid,
                          int isAgree,
                          CB_Func && func);

private:
  // 50020
  void put_map(int32_t sessionid, CB_Func && func);
  void put_map_send(CB_Func && func, Buffer_SP sp);
  void call_erase_map(int32_t sessionid, std::string & key);

  std::mutex sessionid_map_mutex_;
  std::map<int32_t, CB_Func> sessionid_cbfunc_map_;

  void dispatch(int type, Buffer_SP sp);
private:
  // 50030
  void registerRes(Buffer_SP sp);
  void loginRes(Buffer_SP sp);
  void logoutRes(Buffer_SP sp);
  void connectRes(Buffer_SP sp);
  void disconnectRes(Buffer_SP sp);
  void error(Buffer_SP sp);
  void addfriendRes(Buffer_SP sp);
  void addfriendAuthorizeRes(Buffer_SP sp);
private:
  leveldb::DB * db_;
  uint16_t temp_session_;
};


#endif
