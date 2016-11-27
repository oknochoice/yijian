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

typedef struct PingNode Connection_IO;
struct Write_Asyn {
  ev_async as;
};

struct Peer_Servers {
  IPS ips_;
  std::mutex mutex_;
  std::shared_ptr<std::list<PingNode*> > peer_servers_;
};

std::shared_ptr<std::list<PingNode*> > peer_servers();


PingNode * connect_peer(std::string ip, int port);

int start_server_libev(std::vector<std::pair<std::string, int>> ips = 
    std::vector<std::pair<std::string, int>>());


#ifdef __cplusplus
}
#endif

#endif
