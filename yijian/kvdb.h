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
  typedef std::function<void(const std::string&)> CB_Func;

  // 50000 50040
  
  /*
   * common leveldb put get
   *
   * */
  leveldb::Status put(const leveldb::Slice & key, 
      const leveldb::Slice & value);
  leveldb::Status get(const leveldb::Slice & key, 
      std::string & value);

  /*
   * common compose key
   *
   * */
  // u_$userid
  std::string userKey(const std::string & userid);
  // n_$tonodeid
  std::string nodeKey(const std::string & tonodeid);
  // m_$tonodeid_$incrementid
  std::string msgKey(const std::string & tonodeid,
      const std::string & incrementid);
  std::string msgKey(const std::string & tonodeid,
      const int32_t incrementid);
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
   * common current 
   *
   * */
  // current user key = current_user
  void set_current_userid(const std::string & userid);
  std::string get_current_userid();
  void set_current_error(Buffer_SP sp);
  int32_t get_current_error_maxth();//[1, maxth)

  /*
   * common user
   *
   * */
  void putUser(const std::string & userid, 
      const std::string & countrycode,
      const std::string & phoneno,
      const leveldb::Slice & user);
  void getUser(const std::string & id, 
      std::string & user);
  void getUser(const std::string & countrycode,
      const std::string & phoneno,
      std::string & user);


  /*
   * common message
   *
   * */ 
  void putTalklist(const std::string & userid, 
      const leveldb::Slice & talklist);
  void getTalklist(const std::string & userid, 
      std::string & talklist);
  void putMsg(const std::string & msgNode,
      const int32_t & incrementid,
      const leveldb::Slice & msg);
  void getMsg(const std::string & msgNode,
      const int32_t & incrementid,
      std::string & msg);

  // 50010

  /*
   * network regist login logout connect disconnect
   *
   * success or failure
   * func correspond response model
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
   * network friend
   *
   * func
   * failure error key
   * success current user key
   * */
  void addfriend(const std::string & inviteeid,
                 const std::string & msg,
                 CB_Func && func);
  // IsAgree in proto
  void addfriendAuthorize(const std::string & inviterid,
                          int isAgree,
                          CB_Func && func);
  /*
   * network group
   *
   * func
   * failure error key
   * success node key
   *
   * */
  void creategroup(const std::string & groupname,
      const std::vector<std::string> membersid,
      CB_Func && func);
  void addmembers2group(const std::string & groupid,
      const std::vector<std::string> membersid,
      CB_Func && func);
  /*
   * network user
   *
   * func
   * failure error key
   * 
   * */
  // func success version number 
  void queryuserVersion(const std::string & userid,
      CB_Func && func);
  // func success version number 
  void queryuserVersion(CB_Func && func);
  // func success user key
  void queryuser(const std::string && userid, CB_Func && func);
  // func success user key
  void queryuser(CB_Func && func);

  /*
   * network node and message 
   *
   * func
   * failure error key
   *
   * */
  void querynode(const std::string & nodeid,
                 CB_Func && func);
  // type MediaType in proto
  void sendmessage2user(const std::string & userid,
                        const std::string & tonodeid,
                        const int type,
                        const std::string & contenct,
                        CB_Func && func);
  void sendmessage2group(const std::string & tonodeid,
                        const int type,
                        const std::string & contenct,
                        CB_Func && func);
private:
  void querymsg(const std::string & tonodeid,
                const int32_t fromincrementid,
                const int32_t toincrementid,
                CB_Func && func);
  void querymsgContine(CB_Func && func);
public:
  // do not update user's unread info
  void queryonemsg(const std::string & tonodeid,
                   const int32_t incrementid,
                   CB_Func && func);


private:
  // 50020
  void put_map(const int32_t sessionid, CB_Func && func);
  void put_map_send(CB_Func && func, Buffer_SP sp);
  void call_erase_map(const int32_t sessionid, const std::string & key);

  // need delete sessionid as key
  void put_map_send_cache(CB_Func && func, Buffer_SP sp);

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
  void creategroupRes(Buffer_SP sp);
  void groupaddmemberRes(Buffer_SP sp);
  void queryuserversionRes(Buffer_SP sp);
  void queryuserRes(Buffer_SP sp);
  void querynodeRes(Buffer_SP sp);
  void messageRes(Buffer_SP sp);
  void nodemessage(Buffer_SP sp);
private:
  leveldb::DB * db_;
  uint16_t temp_session_;
  int32_t  temp_tonodeid_ = -1;
};


#endif
