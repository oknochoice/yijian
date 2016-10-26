#include "libev_server.h"
#include <map>
#include <function>

#include "message_typemap.h"


List pinglist() {

  YILOG_TRACE ("func: {}. ", __func__);

  static List list = create_pinglist();
  return list;

}

struct ev_loop * loop() {

  YILOG_TRACE ("func: {}. ", __func__);

  static struct ev_loop * loop = EV_DEFAULT;
  return loop;

}

struct ev_io * accept_watcher() {

  YILOG_TRACE ("func: {}. ", __func__);

  static struct ev_io * accept_watcher = 
    (struct ev_io*)malloc(sizeof(struct ev_io));
  return accept_watcher;

}

struct Write_Asyn * write_asyn_watcher() {

  YILOG_TRACE ("func: {}. ", __func__);

  // ev_async
  Write_Asyn * start_write_watcher = reinterpret_cast<Write_Asyn*>(
    malloc(sizeof(struct Write_Asyn)));

  return start_write_watcher;

}


void
sigint_cb (struct ev_loop * loop, ev_signal * w, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  //ev_break(loop, EVBREAK_ALL);
  
}

yijian::noti_threads * noti_threads() {

  YILOG_TRACE ("func: {}. ", __func__);

  static yijian::noti_threads * noti_threads = new yijian::noti_threads();
  return noti_threads;
}

int open_thread_manager () {

  YILOG_TRACE ("func: {}. ", __func__);

  noti_threads();
  
  return 0;
}


int start_server_libev() {

  YILOG_TRACE ("func: {}. ", __func__);

  // manager thread
  if (0 > open_thread_manager()) {
    perror("thread manager open file");
    return -1;
  }

  // signal
  ev_signal signal_watcher;
  ev_signal_init (&signal_watcher, sigint_cb, SIGINT);
  ev_signal_start (loop(), &signal_watcher);

  // ev_async
  ev_async_init(&write_asyn_watcher()->as, start_write_callback);
  ev_set_priority(&write_asyn_watcher()->as, EV_MAXPRI);
  ev_async_start(loop(), &write_asyn_watcher()->as);

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

void start_write_callback (struct ev_loop * loop,  ev_async * w, int revents) {
  
  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  // Write_Asyn * as = reinterpret_cast<Write_Asyn*>(w);

  noti_threads()->foreachio([=](struct PingNode * node) {
      Connection_IO * io = reinterpret_cast<Connection_IO*>(node);
      ev_io_stop(loop, &io->io);
      ev_io_start(loop, &io->contra_io->io);
      });

}

void socket_accept_callback (struct ev_loop * loop, 
    ev_io * rw, int revents) {

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
    accept (rw->fd, (struct sockaddr*) &client_addr, &client_len);
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
  
  // Connection_IO watcher for client
  Connection_IO * client_read_watcher = 
    (Connection_IO *) createReadPingNode();
  Connection_IO * client_write_watcher = 
    (Connection_IO *) createWritePingNode();

  if (NULL == client_read_watcher || 
      NULL == client_write_watcher) {
    perror ("malloc client watcher error");
    return;
  }

  ev_io_init (&client_read_watcher->io, 
      connection_read_callback, client_sd, EV_READ);
  ev_io_init (&client_write_watcher->io,
      connection_read_callback, client_sd, EV_WRITE);
  client_read_watcher->contra_io = client_write_watcher;
  client_write_watcher->contra_io = client_read_watcher;

  ev_io_start (loop, &client_read_watcher->io);

  // add read wather to pinglist
  ping_append(pinglist(), client_read_watcher);
  time(&client_read_watcher->ping_time);

}

void connection_read_callback (struct ev_loop * loop, 
    ev_io * rw, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  Connection_IO * io = reinterpret_cast<Connection_IO*>(rw);

  // read to buffer 
  // if read complete stop watch && update ping
  
  bool isReaded = false;
  
  if (1 == io->buffers_p.size()) {
    isReaded = io->buffers_p.front()->socket_read(io->io.fd);
    if (isReaded) {
      // stop read
      ev_io_stop (loop, rw);
      // update ping time
      time(&io->ping_time);
      ping_move2back(pinglist(), io);
      // do work 
      noti_threads()->sentWork(
          [=](){
            YILOG_TRACE ("dispatch message");
            dispatch(io);
          },
          io,
          [=](){
            YILOG_TRACE ("ev_async_send message");
            ev_async_send(loop, &write_asyn_watcher()->as);
          });
    }
  }else {
    isReaded = io->buffers_p.back()->socket_read_media(io->io.fd);
    if (isReaded) {
      // stop read
      ev_io_stop (loop, rw);
      // update ping time
      time(&io->ping_time);
      ping_move2back(pinglist(), io);
      // do work 
      noti_threads()->sentWork(
          [=](){
            YILOG_TRACE ("dispatch block");
            dispatch(io->buffers_p.back());
          },
          io,
          [=](){
            YILOG_TRACE ("ev_async_send block");
            ev_async_send(loop, &write_asyn_watcher()->as);
          });
    }
  }

}

void connection_write_callback (struct ev_loop * loop, 
    ev_io * ww, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  Connection_IO * io = reinterpret_cast<Connection_IO*>(ww);

  // write to socket
  // if write finish stop write, start read.
  bool isEmpty = io->io_area.buffers_p->empty();
  while (!isEmpty) {
    if (io->io_area.buffers_p->front()->socket_write(io->io.fd)) {
      io->io_area.buffers_p->pop();
    }
  }
  if (isEmpty) {
    io->contra_io->io_area.buffer_sp->reset();
    ev_io_stop(loop, ww);
    ev_io_start(loop, &io->contra_io->io);
  }
}

