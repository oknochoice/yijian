#include "lib_client.h"
#include <ev.h>


struct Connection_IO {
  // watcher
  ev_io io;
  // socket buffer
//  std::mutex buffers_p_mutex;
  std::queue<Buffer_SP> buffers_p;
};

static std::shared_ptr<Read_CB> sp_read_cb_;
static Connection_IO * read_io_;
static Connection_IO * write_io_;

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
    (*sp_read_cb_)(io->buffers_p.front());
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
  if (!io->buffers_p.empty()) {
    auto p = io->buffers_p.front();
    if (p->socket_write(io->io.fd)) {
      io->buffers_p.pop();
    }
  }
  if (io->buffers_p.empty()) {
    ev_io_stop(loop, ww);
  }
}

static void init_io(std::string ip, int port) {
  YILOG_TRACE ("func: {}. ", __func__);
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
  read_io_ = 
    (Connection_IO *) malloc(sizeof(Connection_IO));
  read_io_->buffers_p.front().reset(new yijian::buffer);
  write_io_ = 
    (Connection_IO *) malloc(sizeof(Connection_IO));

  if (NULL == read_io_ || 
      NULL == write_io_) {
    perror ("malloc client watcher error");
  }

  ev_io_init (&read_io_->io, 
      connection_read_callback, sfd, EV_READ);
  ev_io_init (&write_io_->io,
      connection_read_callback, sfd, EV_WRITE);



  ev_io_start (loop(), &read_io_->io);

}

void create_client(Read_CB && read_cb) {
  YILOG_TRACE ("func: {}. ", __func__);
  init_io("127.0.0.1", 5555);
  sp_read_cb_.reset(new Read_CB(std::forward<Read_CB>(read_cb)));
}

void client_send(Buffer_SP sp_buffer) {
  YILOG_TRACE ("func: {}. ", __func__);
  write_io_->buffers_p.push(sp_buffer);
}




