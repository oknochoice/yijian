#ifndef KV_DB_CLIENT_H
#define KV_DB_CLIENT_H

#include <leveldb/db.h>
#include <string>
#include "protofiles/chat_message.pb.h"
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
  // e_$nth
  std::string errorKey(const std::string & nth);
  // t_&userid
  std::string talklistKey(const std::string & userid);
  // signup_kvdb
  std::string signupKey();
  // login_kvdb
  std::string loginKey();
  // connect_kvdb
  std::string connectKey();


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
   * busness
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
             const int OS,
             const std::string & OSversion,
             const std::string & appversion,
             const std::string & devicemodel,
             const std::string & uuid,
             CB_Func && func);
  void connect(const std::string & userid,
               const std::string & uuid,
               const bool isrecivenoti,
               CB_Func && func);
private:
  void put_map(int32_t sessionid, CB_Func && func);
  void call_erase_map(int32_t sessionid, std::string & key);

  std::mutex sessionid_map_mutex_;
  std::map<int32_t, CB_Func> sessionid_cbfunc_map_;

  void dispatch(int type, Buffer_SP sp);
private:
  void registerRes(Buffer_SP sp);
  void loginRes(Buffer_SP sp);
  void connectRes(Buffer_SP sp);
private:
  leveldb::DB * db_;
  unsigned int16_t sessionid_;
};


#endif
