#ifndef LIBEV_SERVER_H_
#define LIBEV_SERVER_H_
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "time.h"
#include "pinglist.h"
#include "threads_work.h"

#include <signal.h>
#include <ev.h>
#include <netinet/in.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#define PORT 5555
#define MAX_CONNECTIONS 100

typedef std::vector<std::pair<std::string, int>> IPS;

struct Write_IO;

struct Read_IO {
  struct PingNode pingnode;
  struct Write_IO * writeio;
  // socket buffer
  Buffer_SP buffer_sp = std::make_shared<yijian::buffer>();
  // node info
  uint16_t sessionid;
  bool     isConnect = false;
  std::string userid;
  std::string deviceid;
  std::string appVersion;
  std::string clientVersion;
  std::vector<chat::Media> media_vec;
  std::pair<std::string, std::shared_ptr<mongocxx::cursor>> id_cursor;
};

struct Write_IO {
  struct ev_io io;
  struct Read_IO * readio;
  // socket buffer
  std::mutex buffers_p_mutex;
  std::queue<Buffer_SP> buffers_p;
};

struct Write_Asyn {
  ev_async as;
};

struct Peer_Servers {
  IPS ips_;
  std::mutex mutex_;
  std::shared_ptr<std::list<Read_IO*> > peer_servers_;
};

std::shared_ptr<std::list<Read_IO*> > peer_servers();


Read_IO * connect_peer(std::string ip, int port);

int start_server_libev(std::vector<std::pair<std::string, int>> ips = 
    std::vector<std::pair<std::string, int>>());


#ifdef __cplusplus
}
#endif

#endif
