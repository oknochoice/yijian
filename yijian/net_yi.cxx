#include "net_yi.h"
#include "macro.h"
#include "typemap.h"
#include "buffer_yi.h"

using yijian::buffer;

enum Session_ID : int32_t {
  regist_login_connect = -1,
  logout_disconnect = -2,
  accept_unread_msg = -3,
  user_noti = -4
};

netyi::netyi() {
  YILOG_TRACE ("func: {}", __func__);

}

netyi::~netyi() {
  YILOG_TRACE ("func: {}", __func__);
  clear_client();
}
/*
 * net func
 * */
void netyi::net_connect() {
  YILOG_TRACE ("func: {}", __func__);
  create_client([&](Buffer_SP sp){
    YILOG_TRACE ("net callback");
    switch(sp->datatype()) {
      case ChatType::loginnoti:
      case ChatType::addfriendnoti:
      case ChatType::addfriendauthorizenoti:
      {
        call_map(Session_ID::user_noti, sp);
        break;
      }
      case ChatType::nodemessagenoti:
      {
        call_map(Session_ID::accept_unread_msg, sp);
        break;
      }
      case ChatType::registorres:
      case ChatType::loginres:
      case ChatType::clientconnectres:
      {
        call_map(Session_ID::regist_login_connect, sp);
        break;
      }
      case ChatType::logoutres:
      case ChatType::clientdisconnectres:
      {
        call_map(Session_ID::logout_disconnect, sp);
        break;
      }
      default:
      call_map(sp->session_id(), sp);
    }

  });
}


void netyi::signup_login_connect(Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map_send(Session_ID::regist_login_connect, sp,
      std::forward<CB_Func_Mutiple>(func));
}

void netyi::logout_disconnect(Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map_send(Session_ID::logout_disconnect, sp,
      std::forward<CB_Func_Mutiple>(func));
}
void netyi::send_buffer(Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map_send(sp,
      std::forward<CB_Func_Mutiple>(func));
}

/*
 * net noti
 *
 * */
// unread msg noti func(key)
void netyi::acceptUnreadMsg(CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map(Session_ID::accept_unread_msg, 
      std::forward<CB_Func_Mutiple>(func));
}
// other device loginNoti addFriendNoti addFriendAuthorizeNoti
void netyi::userInfoNoti(CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  put_map(Session_ID::user_noti, 
      std::forward<CB_Func_Mutiple>(func));
}

/*
 * private
 * */
void netyi::put_map(const int32_t sessionid, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  sessionid_cbfunc_map_[sessionid] = func;
}
void netyi::put_map_send(Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  uint16_t temp_session;
  client_send(sp, &temp_session);
  YILOG_TRACE ("func: {}, sessionid: {}", __func__, temp_session);
  sessionid_cbfunc_map_[temp_session] = func;
}
void netyi::put_map_send(const int32_t sessionid,
    Buffer_SP sp, CB_Func_Mutiple && func) {
  YILOG_TRACE ("func: {}", __func__);
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  client_send(sp, nullptr);
  sessionid_cbfunc_map_[sessionid] = func;
}
bool netyi::call_map(const int32_t sessionid, Buffer_SP sp) {
  YILOG_TRACE ("func: {}", __func__);
  bool isCalled = false;
  auto lfunc = CB_Func_Mutiple();
  std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
  auto it = sessionid_cbfunc_map_.find(sessionid);
  if (likely(it != sessionid_cbfunc_map_.end())) {
    lfunc = it->second;
  }else {
    YILOG_DEBUG ("user stop call back");
  }
  ul.unlock();
  if (lfunc) {
    bool isStop = true;
    lfunc(sp->datatype(), sp->data(), sp->data_size(), &isStop);
    isCalled = true;
    if (isStop) {
      std::unique_lock<std::mutex> ul(sessionid_map_mutex_);
      sessionid_cbfunc_map_.erase(it);
    }
  }
  return isCalled;
}

