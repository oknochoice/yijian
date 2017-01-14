#ifndef LIBEV_SERVER_H_
#define LIBEV_SERVER_H_
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "time.h"
#include "threads_work.h"

#include <signal.h>
#include <ev.h>
#include <netinet/in.h>
#include <list>
#include <string>
#include <chrono>
#include "buffer_yi.h"
#include "libev_server.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PORT 5555
#define MAX_CONNECTIONS 100


typedef std::shared_ptr<yijian::buffer> Buffer_SP;

typedef std::vector<std::pair<std::string, int>> IPS;

typedef std::shared_ptr<Read_IO> Read_IO_SP;
typedef std::list<Read_IO_SP> Imp_list;
typedef Imp_list::iterator Iter;

struct Write_IO;
struct Read_IO {
  struct ev_io io;
  //
  uint64_t ping_time;// seconds
  std::shared_ptr<Iter> iter_sp;
  //
  std::shared_ptr<Write_IO> writeio_sp;
  std::weak_ptr<Read_IO> self;
  // socket buffer subthread must not change
  Buffer_SP buffer_sp = std::make_shared<yijian::buffer>();
  // node info
  // connect info subthread must not change
  bool isConnect = false;
  // set value where connectStatus is login_in connect_in
  uint16_t sessionid;
  std::string userid;
  std::string uuid;
  std::string appVersion;
  std::string clientVersion;
  // media need 
  std::mutex media_vec_mutex_;
  std::vector<chat::Media> media_vec;
};

struct Write_IO {
  struct ev_io io;
  std::weak_ptr<Read_IO> readio_sp;
  std::weak_ptr<Write_IO> self;
  // socket buffer
  std::mutex buffers_p_mutex;
  std::queue<Buffer_SP> buffers_p;
};

struct Write_Asyn {
  ev_async as;
};

void mountBuffer2Device(const std::string & uuid, 
    std::function<void(std::shared_ptr<Read_IO>)> && func);


void peer_server_foreach(
    std::function<void(std::shared_ptr<Read_IO>)> && func);

std::shared_ptr<Read_IO> connect_peer(std::string ip, int port);

int start_server_libev(std::vector<std::pair<std::string, int>> ips = 
    std::vector<std::pair<std::string, int>>());


#ifdef __cplusplus
}
#endif

#endif
