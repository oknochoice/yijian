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

typedef struct PingNode Connection_IO;
struct Write_Asyn {
  ev_async as;
};

struct Peer_Servers {
std::mutex mutex_;
std::shared_ptr<std::list<PingNode*> > peer_servers_;
}

std::shared_ptr<std::list<PingNode*>> peer_servers();

static List pinglist();
static struct ev_loop * loop();
static struct ev_io * accept_watcher();
static struct Write_Asyn * write_asyn_watcher();
static yijian::noti_threads * noti_treads();


static void
sigint_cb (struct ev_loop * loop, ev_signal * w, int revents);
// ev_async watcher priority si EV_MAXPRI
static void
start_write_callback (struct ev_loop * loop, ev_async * w, int revents);
static void
socket_accept_callback (struct ev_loop * loop, ev_io * rw, int revents);
static void
connection_read_callback (struct ev_loop * loop, ev_io * rw, int revents);
static void
connection_write_callback (struct ev_loop * loop, ev_io * ww, int revents);

PingNode * connect_peer(std::string ip, int port);

int start_server_libev(std::vector<std::pair<std::string, int>> & ips );


#ifdef __cplusplus
}
#endif

#endif
