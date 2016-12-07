#include "lib_client.h"
#include <ev.h>
#include <utility>
#include <queue>

struct Connection_IO {
  // watcher
  ev_io io;
  // socket buffer
  std::mutex buffers_p_mutex;
  std::queue<Buffer_SP> buffers_p;
};

static std::shared_ptr<Read_CB> sp_read_cb_;
static Connection_IO * read_io_;
static Connection_IO * write_io_;

struct ev_loop * loop() {

  YILOG_TRACE ("func: {}. ", __func__);

  static struct ev_loop * loop = ev_loop_new(0);
  return loop;

}

struct ev_async * write_asyn_watcher() {

  YILOG_TRACE ("func: {}. ", __func__);

  // ev_async
  static struct ev_async * start_write_watcher = reinterpret_cast<struct ev_async*>(
    malloc(sizeof(struct ev_async)));

  return start_write_watcher;

}

void start_write_callback (struct ev_loop * loop,  ev_async * r, int revents) {
  
  YILOG_TRACE ("func: {}. ", __func__);
  //ev_io_stop(loop, &read_io_->io);
  ev_io_start(loop, &write_io_->io);

}

void connection_read_callback (struct ev_loop * loop, 
    ev_io * rw, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  ev_io_stop(loop, rw);
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
  std::unique_lock<std::mutex> ul(io->buffers_p_mutex);
  if (!io->buffers_p.empty()) {
    auto p = io->buffers_p.front();
    ul.unlock();
    if (p->socket_write(io->io.fd)) {
      ul.lock();
      io->buffers_p.pop();
      ul.unlock();
    }
  }else {
    YILOG_TRACE ("func: {}. stop write", __func__);
    ev_io_stop(loop, ww);
  }
  YILOG_TRACE ("func: {}. write finish", __func__);
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
    return ;
  }

  YILOG_TRACE("Connected Success {}:{}.", ip.data(), port);

  // ev_async
  struct ev_async * async_io = write_asyn_watcher();
  ev_async_init(async_io, start_write_callback);
  ev_set_priority(async_io, EV_MAXPRI);
  ev_async_start(loop(), async_io);
  YILOG_TRACE("ev_async init Success");

  // Connection_IO watcher for client
  read_io_ = new Connection_IO();
  read_io_->buffers_p.push(std::make_shared<yijian::buffer>());
  write_io_ = new Connection_IO();

  if (NULL == read_io_ || 
      NULL == write_io_) {
    perror ("new client watcher error");
  }

  ev_io_init (&read_io_->io, 
      connection_read_callback, sfd, EV_READ);
  ev_io_init (&write_io_->io,
      connection_write_callback, sfd, EV_WRITE);

  ev_io_start (loop(), &read_io_->io);
  YILOG_TRACE("read watcher write watcher init Success");

}

void create_client(Read_CB && read_cb) {
  YILOG_TRACE ("func: {}. ", __func__);
  std::thread t([&](){
    YILOG_TRACE ("func: {}, thread start.", __func__);
    init_io("127.0.0.1", 5555);
    sp_read_cb_.reset(new Read_CB(std::forward<Read_CB>(read_cb)));
    ev_run(loop(), 0);
    YILOG_TRACE("exit thread");
      });
  t.detach();
}

void client_send(Buffer_SP sp_buffer) {
  YILOG_TRACE ("func: {}. ", __func__);

  std::unique_lock<std::mutex> ul(write_io_->buffers_p_mutex);
  write_io_->buffers_p.push(sp_buffer);
  auto watcher = write_asyn_watcher();
  ev_async_send(loop(), watcher);

}





