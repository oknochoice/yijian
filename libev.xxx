#define CATCH_CONFIG_RUNNER
#define YILOG_ON
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "time.h"

#include <signal.h>
#include <ev.h>
#include <netinet/in.h>

#include <list>

#define PORT 5555
#define MAX_CONNECTIONS 100

struct yi_io {
  ev_io io;
  time_t last_recv_msg;
};

struct yi_timer {
  ev_timer timer;
};

static std::list<yi_io*>&
fd_list() {
  auto static list = std::list<yi_io*>();
  return list;
}

static void
sigint_cb (struct ev_loop * loop, ev_signal * w, int revents) {
  printf("deal SIGINT\n");
  //ev_break(loop, EVBREAK_ALL);
}

static void
socket_accept_callback (struct ev_loop * loop, ev_io * w, int revents);
static void
socket_read_callback (struct ev_loop * loop, ev_io * w, int revents);


int main (int argc, char * argv[])
{
  // ev loop
  struct ev_loop * loop = EV_DEFAULT;
  // sigint 
  ev_signal signal_watcher;
  ev_signal_init (&signal_watcher, sigint_cb, SIGINT);
  ev_signal_start (loop, &signal_watcher);

  // socket start
  int sd;
  struct sockaddr_in addr;
  int addr_len = sizeof(addr);

  struct ev_io * socket_watcher = (struct ev_io*)malloc(sizeof(struct ev_io));

  sd = socket(PF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    perror("socket error");
    return -1;
  }
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = INADDR_ANY;
  // bind
  if (0 != bind(sd, (struct sockaddr*) &addr, sizeof(addr))) {
    perror("bind error");
    return -1;
  }
  // listen
  if (listen(sd, SOMAXCONN) < 0) {
    perror("listen error");
  }
  // set sd reuse
  int reuseaddr = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char*) &reuseaddr, sizeof(reuseaddr)) != 0) {
    perror("set socket reuse addr error");
    return -1;
  }

  ev_io_init(socket_watcher, socket_accept_callback, sd, EV_READ);
  ev_io_start(loop, socket_watcher);

  ev_run (loop, 0);

  return 0;
}

void socket_accept_callback (struct ev_loop * loop, ev_io * w, int revents) {

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sd;

  // accept w->fd
  if (EV_ERROR & revents) {
    perror ("error event in accept");
    return;
  }

  client_sd = accept (w->fd, (struct sockaddr*) &client_addr, &client_len);
  if (client_sd < 0) {
    perror ("accept error");
    return;
  }
  if (client_sd > MAX_CONNECTIONS) {
    perror ("fd too large");
    close(client_sd);
    return;
  }
  
  printf ("client connected\n");
  
  // yi_io watcher for client
  struct yi_io * client_watcher = (struct yi_io *) malloc(sizeof(struct yi_io));

  if (NULL == client_watcher) {
    perror ("malloc client watcher error");
    return;
  }

  ev_io_init (&client_watcher->io, socket_read_callback, client_sd, EV_READ);
  ev_io_start (loop, &client_watcher->io);

  time (&client_watcher->last_recv_msg);

  YILOG_INFO ("connect time is {}", client_watcher->last_recv_msg);

  fd_list().push_back(client_watcher);

}

void socket_read_callback (struct ev_loop * loop, ev_io * w, int revents) {

}


