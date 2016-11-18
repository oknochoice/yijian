#include "lib_client.h"
#include <ev.h>
#include "buffer.h"

struct Connection_IO {
  // watcher
  ev_io io;
  // if io is read, contra_io is write
  // if io is write, contra_io io read
  struct Connection_IO * contra_io;
  // socket buffer
  std::mutex buffers_p_mutex;
  std::queue<Buffer_SP> buffers_p;
};

struct ev_loop * loop() {

  YILOG_TRACE ("func: {}. ", __func__);

  static struct ev_loop * loop = EV_DEFAULT;
  return loop;

}

void connection_read_callback (struct ev_loop * loop, 
    ev_io * rw, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  Connection_IO * io = reinterpret_cast<Connection_IO*>(rw);

  // read to buffer 
  // if read complete stop watch && update ping
  
  if (io->buffers_p.front()->socket_read(io->io.fd)) {
    // stop read
    ev_io_stop (loop, rw);
    // update ping time
    time(&io->ping_time);
    ping_move2back(pinglist(), io);
    if (unlikely(io->buffers_p.front()->datatype() == 
          ChatType::serverconnect)) {
      // remove peer server's pingnode from pinglist
      // add to peer server list;
      ping_erase(pinglist(), io);
      auto p = peer_servers_write();
      p->push_back(io);
    }else if (unlikely(io->buffers_p.front()->datatype() == 
          ChatType::serverdisconnect)) {
      // remove peer server list;
      auto p = peer_servers_write();
      p->remove(io);
    }else {
      // do work 
      noti_threads()->sentWork(
          [=](){
            YILOG_TRACE ("dispatch message");
            dispatch(io, io->buffers_p.front());
            ev_async_send(loop, &write_asyn_watcher()->as);
          });
    }
    io->buffers_p.front().reset(new yijian::buffer());
  }

}

void connection_write_callback (struct ev_loop * loop, 
    ev_io * ww, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  Connection_IO * io = reinterpret_cast<Connection_IO*>(ww);

  // write to socket
  // if write finish stop write, start read.
  std::unique_lock<std::mutex> ul(io->buffers_p_mutex);
  if (!io->buffers_p.empty()) {
    auto p = io->buffers_p.front();
    ul.unlock();
    if (p->socket_write(io->io.fd)) {
      ul.lock();
      io->buffers_p.pop();
      ul.unlock();
    }
  }
  if (io->buffers_p.empty()) {
    ul.unlock();
    ev_io_stop(loop, ww);
  }
}

Connection_IO * connect_peer(std::string ip, int port) {
  int sfd;
  struct sockaddr_in addr;
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sfd < 0) {
    perror("socket error");
  }

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip.data());

  if (connect(sfd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
    perror("Failed to connect server");
  }

  printf("Connected Success %s:%d", ip.data(), port);

  // Connection_IO watcher for client
  Connection_IO * client_read_watcher = 
    (Connection_IO *) malloc(sizeof(Connection_IO));
  Connection_IO * client_write_watcher = 
    (Connection_IO *) malloc(sizeof(Connection_IO));

  if (NULL == client_read_watcher || 
      NULL == client_write_watcher) {
    perror ("malloc client watcher error");
  }

  ev_io_init (&client_read_watcher->io, 
      connection_read_callback, sfd, EV_READ);
  ev_io_init (&client_write_watcher->io,
      connection_read_callback, sfd, EV_WRITE);
  client_read_watcher->contra_io = client_write_watcher;
  client_write_watcher->contra_io = client_read_watcher;

  ev_io_start (loop(), &client_read_watcher->io);

  return client_read_watcher;
}

void * create_client() {
  Connection_IO* node = connect_peer("127.0.0.1", 5555);
}




