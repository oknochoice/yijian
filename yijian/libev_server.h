#ifndef LIBEV_SERVER_H_
#define LIBEV_SERVER_H_
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "time.h"
#include "pinglist.h"

#include <signal.h>
#include <ev.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PORT 5555
#define MAX_CONNECTIONS 100

typedef struct PingNode Connection_io;

static List& pinglist() {
  static List list = create_pinglist();
  return list;
}

static struct ev_loop*  & loop() {
  static struct ev_loop * loop = EV_DEFAULT;
  return loop;
}

static struct ev_io *& accept_watcher() {
  static struct ev_io * accept_watcher = 
    (struct ev_io*)malloc(sizeof(struct ev_io));
  return accept_watcher;
}

static void
sigint_cb (struct ev_loop * loop, ev_signal * w, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  //ev_break(loop, EVBREAK_ALL);
  
}

static void
socket_accept_callback (struct ev_loop * loop, ev_io * w, int revents);
static void
connection_read_callback (struct ev_loop * loop, ev_io * w, int revents);
static void
connection_write_callback (struct ev_loop * loop, ev_io * w, int revents);


int start_server_libev() {

  YILOG_TRACE ("func: {}. ", __func__);

  // signal
  ev_signal signal_watcher;
  ev_signal_init (&signal_watcher, sigint_cb, SIGINT);
  ev_signal_start (loop(), &signal_watcher);

  // socket 
  int sd;
  struct sockaddr_in addr;
  int addr_len = sizeof(addr);

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
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, 
        (const char*) &reuseaddr, sizeof(reuseaddr)) != 0) {
    perror("set socket reuse addr error");
    return -1;
  }

  ev_io_init(accept_watcher(), socket_accept_callback, sd, EV_READ);
  ev_io_start(loop(), accept_watcher());

  ev_run (loop(), 0);

  return 0;

}

void socket_accept_callback (struct ev_loop * loop, 
    ev_io * w, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sd;

  // accept w->fd
  if (EV_ERROR & revents) {
    perror ("error event in accept");
    return;
  }

  client_sd = 
    accept (w->fd, (struct sockaddr*) &client_addr, &client_len);
  if (client_sd < 0) {
    perror ("accept error");
    return;
  }
  if (client_sd > MAX_CONNECTIONS) {
    perror ("fd too large");
    close(client_sd);
    return;
  }
  
  YILOG_INFO ("client connected");
  
  // Connection_io watcher for client
  Connection_io * client_read_watcher = 
    (Connection_io *) malloc(sizeof(Connection_io));
  Connection_io * client_write_watcher = 
    (Connection_io *) malloc(sizeof(Connection_io));

  if (NULL == client_read_watcher || 
      NULL == client_write_watcher) {
    perror ("malloc client watcher error");
    return;
  }

  ev_io_init (&client_read_watcher->io, 
      connection_read_callback, client_sd, EV_READ);
  ev_io_init (&client_write_watcher->io,
      connection_read_callback, client_sd, EV_WRITE);
  ev_io_start (loop, &client_read_watcher->io);

  // add read wather to pinglist
  ping_append(pinglist(), client_read_watcher);
  time(&client_read_watcher->ping_time);

}

void connection_read_callback (struct ev_loop * loop, 
    ev_io * w, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  Connection_io * io = reinterpret_cast<Connection_io*>(w);

  // read to buffer 
  
  // if read complete stop watch && update ping
  ev_io_stop (loop, w);

  time(&io->ping_time);
  ping_move2back(pinglist(), io);
  // do work 

}

void connection_write_callback (struct ev_loop * loop, 
    ev_io * w, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  Connection_io * io = reinterpret_cast<Connection_io*>(w);

  // write to socket
  
  // if write finish close write
  ev_io_stop(loop, w);
  
}


#ifdef __cplusplus
}
#endif

#endif
