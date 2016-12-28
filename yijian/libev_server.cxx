#include "libev_server.h"
#include <map>
#include <functional>
#include <mutex>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <buffer.h>
#include <list>

#include "server_msg_typemap.h"
/*
 *
 * priate declare
 *
 * */

static struct ev_loop * loop();
static struct ev_io * accept_watcher();
static struct ev_timer * timer_watcher();
static struct Write_Asyn * write_asyn_watcher();
static yijian::noti_threads * noti_threads();

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

static bool isStopThreadFunc_ = false;

/*
 * ping list
 *
 * */

uint64_t ping_time() {
  YILOG_TRACE ("func: {}. ", __func__);
  auto now = std::chrono::high_resolution_clock::now();
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
      now.time_since_epoch());
  return seconds.count();
}
Imp_list pinglist_;
void ping_append(Read_IO_SP sp)  {
  YILOG_TRACE ("func: {}. ", __func__);
  sp->iter = pinglist_.insert(pinglist_.end(), sp);
  sp->ping_time = ping_time();
}
void ping_move2back(Read_IO_SP sp) {
  YILOG_TRACE ("func: {}. ", __func__);
  pinglist_.erase(sp->iter);
  sp->iter = pinglist_.insert(pinglist_.end(), sp);
  sp->ping_time = ping_time();
}
void ping_erase(Read_IO_SP sp) {
  YILOG_TRACE ("func: {}. ", __func__);
  pinglist_.erase(sp->iter);
}
void ping_foreach(std::function<void(Read_IO_SP, bool * )> && func) {
  YILOG_TRACE ("func: {}. ", __func__);
  bool isStop = false;
  for (auto & io: pinglist_) {
    if (true == isStop) {
      break;
    }
    func(io, &isStop);
  }
}

static void 
pingtime_callback(EV_P_ ev_timer *w, int revents) {
  YILOG_TRACE ("func: {}. ", __func__);
  ev_timer_stop(loop, w);
  uint64_t now = ping_time();
  ping_foreach([now](Read_IO_SP sp, bool * isStop){
        if (now - sp->ping_time > 120) {
          ping_erase(sp);
          *isStop = false;
        }else {
          *isStop = true;
        }
      });
  ev_timer_set (w, 120., 0.);

  if (unlikely(isStopThreadFunc_ == true)) {
    auto accept_io = accept_watcher();
    ev_io_stop(loop, accept_io);
    for (auto io: pinglist_) {
      ev_io_stop(loop, &io->io);
    }
    ev_timer_set (w, 1., 0.);
    if (noti_threads()->taskCount()) {
      ev_async_stop(loop, &write_asyn_watcher()->as);
      delete noti_threads();
      free   (timer_watcher());
      free   (accept_watcher());
      delete write_asyn_watcher();
      ev_loop_destroy(loop);
      exit(0);
    }
  }
  ev_timer_start (loop, w);
}
/*
 *
 * uuid key read io pointor value: map
 *
 *
 * */ 
std::mutex uuidnode_map_mutex_;
std::unordered_map<std::string, std::shared_ptr<Read_IO>> uuidnode_map_;

void uuidnode_put(const std::string & uuid, std::shared_ptr<Read_IO> io) {
  YILOG_TRACE ("func: {}. ", __func__);
  std::unique_lock<std::mutex> ul(uuidnode_map_mutex_);
  uuidnode_map_[uuid] = io;
}

void uuidnode_delete(const std::string & uuid) {
  YILOG_TRACE ("func: {}. ", __func__);
  std::unique_lock<std::mutex> ul(uuidnode_map_mutex_);
  uuidnode_map_.erase(uuid);
}

void mountBuffer2Device(Buffer_SP sp, const std::string & uuid) {
  YILOG_TRACE ("func: {}. ", __func__);
  std::unique_lock<std::mutex> ul(uuidnode_map_mutex_);
  auto it = uuidnode_map_.find(uuid);
  if (it != uuidnode_map_.end()) {
    {
      std::unique_lock<std::mutex> uiol(
          it->second->writeio_sp->buffers_p_mutex);
      it->second->writeio_sp->buffers_p.push(sp);
    }
    yijian::threadCurrent::pushPingnode(it->second);
  }
}
/*
 * peer server manage
 *
 *
 * */
typedef std::shared_ptr<std::list<std::shared_ptr<Read_IO>>> IO_List;
struct Peer_Servers {
  IPS ips_;
  std::mutex mutex_;
  IO_List io_list_;
};

struct Peer_Servers peer_servers_;

void peer_server_foreach(
    std::function<void(std::shared_ptr<Read_IO>)> && func) {

  YILOG_TRACE ("func: {}. ", __func__);

  IO_List list;
  {
    std::unique_lock<std::mutex> ul(peer_servers_.mutex_);
    list = peer_servers_.io_list_;
  }
  for (std::shared_ptr<Read_IO> sp: *list) {
    func(sp);
  }

}

void peer_servers_push(std::shared_ptr<Read_IO> io) {

  YILOG_TRACE ("func: {}. ", __func__);

  std::unique_lock<std::mutex> ul(peer_servers_.mutex_);
  if (!peer_servers_.io_list_.unique()) {
    peer_servers_.io_list_.reset(new 
        std::list<std::shared_ptr<Read_IO>>(*peer_servers_.io_list_));
  }
  io->iter = peer_servers_.io_list_->insert(peer_servers_.io_list_->end(), io);
}

void peer_servers_erase(std::shared_ptr<Read_IO> io) {
  YILOG_TRACE ("func: {}. ", __func__);
  std::unique_lock<std::mutex> ul(peer_servers_.mutex_);
  if (!peer_servers_.io_list_.unique()) {
    peer_servers_.io_list_.reset(new 
        std::list<std::shared_ptr<Read_IO>>(*peer_servers_.io_list_));
  }
  peer_servers_.io_list_->erase(io->iter);
}

std::shared_ptr<Read_IO> connect_peer(std::string ip, int port) {
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
  auto client_read_watcher = std::make_shared<Read_IO>();
  auto client_write_watcher = std::make_shared<Write_IO>();
  client_read_watcher->writeio_sp = client_write_watcher;
  client_read_watcher->self = client_read_watcher;
  client_write_watcher->readio_sp = client_read_watcher;
  client_write_watcher->self = client_write_watcher;

  if (NULL == client_read_watcher || 
      NULL == client_write_watcher) {
    perror ("new client watcher error");
  }

  ev_io_init (&client_read_watcher->io, 
      connection_read_callback, sfd, EV_READ);
  ev_io_init (&client_write_watcher->io,
      connection_read_callback, sfd, EV_WRITE);

  ev_io_start (loop(), &client_read_watcher->io);

  return client_read_watcher;
}



static struct ev_loop * loop() {

  YILOG_TRACE ("func: {}. ", __func__);

  static struct ev_loop * loop = ev_default_loop(0);
  return loop;

}

static struct ev_io * accept_watcher() {

  YILOG_TRACE ("func: {}. ", __func__);

  static struct ev_io * accept_watcher = 
    (struct ev_io*)malloc(sizeof(struct ev_io));
  return accept_watcher;

}

static struct ev_timer * timer_watcher() {
  YILOG_TRACE ("func: {}. ", __func__);
  struct ev_timer * timer = 
    (struct ev_timer*)malloc(sizeof(struct ev_timer));
  return timer;
}

static struct Write_Asyn * write_asyn_watcher() {

  YILOG_TRACE ("func: {}. ", __func__);

  // ev_async
  static Write_Asyn * start_write_watcher = new Write_Asyn();

  return start_write_watcher;

}



static yijian::noti_threads * noti_threads() {

  YILOG_TRACE ("func: {}. ", __func__);

  static yijian::noti_threads * noti_threads = new yijian::noti_threads(1);
  return noti_threads;
}




int open_thread_manager () {

  YILOG_TRACE ("func: {}. ", __func__);

  noti_threads();
  
  return 0;

}
static void
sigint_cb (struct ev_loop * loop, ev_signal * w, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

#warning fixed me
  // manager thread
  if (0 > open_thread_manager()) {
    perror("thread manager open file");
    return;
  }

  // connect to peer server
  for (auto & pair: peer_servers_.ips_) {
    peer_servers_push(connect_peer(pair.first, pair.second));
  }
  // reclaim resource
  // stop server
  if (false) {
    isStopThreadFunc_ = true;
  }
}


static void
sigusr1_cb (struct ev_loop * loop, ev_signal * w, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);
  
}
void initSetup(IPS & ips) {
  YILOG_TRACE ("func: {}. ", __func__);
  // set peer server list
  peer_servers_.io_list_.reset(new std::list<std::shared_ptr<Read_IO>>());
  peer_servers_.ips_ = ips;
}

int start_server_libev(IPS ips ) {

  YILOG_TRACE ("func: {}. ", __func__);
  initSetup(ips);
  // signal
  ev_signal signal_int_watcher;
  ev_signal_init (&signal_int_watcher, sigint_cb, SIGINT);
  ev_signal_start (loop(), &signal_int_watcher);

  ev_signal signal_user1_watcher;
  ev_signal_init(&signal_user1_watcher, sigusr1_cb, SIGUSR1);
  ev_signal_start(loop(), &signal_user1_watcher);

  // ev_async
  struct ev_async * async_io = &write_asyn_watcher()->as;
  ev_async_init(async_io, start_write_callback);
  ev_set_priority(async_io, EV_MAXPRI);
  ev_async_start(loop(), async_io);

  // socket 
  int sd;
  struct sockaddr_in addr;
  int addr_len = sizeof(addr);

  sd = socket(PF_INET, SOCK_STREAM, 0);
  int flags = fcntl(sd, F_GETFL, 0);
  fcntl(sd, F_SETFL, flags | O_NONBLOCK);
  if (sd < 0) {
    perror("socket error");
    return -1;
  }
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = INADDR_ANY;
  // set sd reuse
  int reuseaddr = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, 
        (const char*) &reuseaddr, sizeof(reuseaddr)) != 0) {
    perror("set socket reuse addr error");
    return -1;
  }
  // bind
  if (0 != bind(sd, (struct sockaddr*) &addr, sizeof(addr))) {
    perror("bind error");
    return -1;
  }
  // listen
  if (listen(sd, SOMAXCONN) < 0) {
    perror("listen error");
  }

  struct ev_io * accept_io = accept_watcher();
  ev_io_init(accept_io, socket_accept_callback, sd, EV_READ);
  ev_io_start(loop(), accept_io);


  //timer ping
  struct ev_timer * timer = timer_watcher();
  ev_timer_init (timer, pingtime_callback, 120., 0.);
  ev_timer_start (loop(), timer);

  ev_run (loop(), 0);

  YILOG_TRACE ("exit main");
  return 0;

}



static void 
start_write_callback (struct ev_loop * loop,  ev_async * r, int revents) {
  
  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  // Write_Asyn * as = reinterpret_cast<Write_Asyn*>(w);

  noti_threads()->foreachio([=](std::shared_ptr<Read_IO> io) {
        YILOG_TRACE ("func: noti_threads foreachio. ");
        ev_io_start(loop, &io->io);
        ev_io_start(loop, &io->writeio_sp->io);
      });

}

static void 
socket_accept_callback (struct ev_loop * loop, 
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

  int flags = fcntl(client_sd, F_GETFL, 0);
  fcntl(client_sd, F_SETFL, flags | O_NONBLOCK);
  
  YILOG_INFO ("client connected");
  
  // Connection_IO watcher for client
  auto client_read_watcher = std::make_shared<Read_IO>();
  auto client_write_watcher = std::make_shared<Write_IO>();
  client_read_watcher->writeio_sp = client_write_watcher;
  client_read_watcher->self = client_read_watcher;
  client_write_watcher->readio_sp = client_read_watcher;
  client_write_watcher->self = client_write_watcher;

  if (NULL == client_read_watcher || 
      NULL == client_write_watcher) {
    perror ("new client watcher error");
    return;
  }

  ev_io_init (&client_read_watcher->io, 
      connection_read_callback, client_sd, EV_READ);
  ev_io_init (&client_write_watcher->io,
      connection_write_callback, client_sd, EV_WRITE);

  ev_io_start (loop, &client_read_watcher->io);

  // add read wather to pinglist
  ping_append(client_read_watcher);

}

static void 
connection_read_callback (struct ev_loop * loop, 
    ev_io * rw, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  ev_io_stop(loop, rw);

  // converse to usable io
  Read_IO * io = reinterpret_cast<Read_IO*>(rw);
  auto io_sp = io->self.lock();
  if(io_sp) {

    // read to buffer 
    // if read complete stop watch && update ping
    
    if (io->buffer_sp->socket_read(io->io.fd)) {
      // update ping time
      YILOG_TRACE ("func: {}. update ping time", __func__);
      ping_move2back(io_sp);
      // server connect
      if (unlikely(io->buffer_sp->datatype() == 
            ChatType::serverconnect)) {
        // remove peer server's pingnode from pinglist
        // add to peer server list;
        YILOG_TRACE ("func: {}. process peer sever connect", __func__);
        ping_erase(io_sp);
        peer_servers_push(io_sp);
      // server disconnect
      }else if (unlikely(io->buffer_sp->datatype() == 
            ChatType::serverdisconnect)) {
        // remove peer server list;
        YILOG_TRACE ("func: {}. process peer sever disconnect", __func__);
        peer_servers_erase(io_sp);
      }else {
        // do work 
        YILOG_TRACE ("func: {}. subthread do work", __func__);
        auto sp = io->buffer_sp;
        auto watcher = &write_asyn_watcher()->as;
        uint16_t sessionid = io->sessionid++;
        noti_threads()->sentWork(
            [io_sp, &loop, watcher, sp, sessionid](){
              YILOG_TRACE ("dispatch message");
              dispatch(io_sp, sp, sessionid);
              ev_async_send(loop, watcher);
            });
      }
      
      YILOG_TRACE("buffer sp reference count {}", 
          io->buffer_sp.use_count());
      io->buffer_sp.reset(new yijian::buffer());
    }else{
      YILOG_TRACE ("read is not complete message");
    }
  }else {
    YILOG_TRACE ("node is released");
  }

}

static void 
connection_write_callback (struct ev_loop * loop, 
    ev_io * ww, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  Write_IO * io = reinterpret_cast<Write_IO*>(ww);
  auto io_sp = io->readio_sp.lock();
  if (io_sp) {
    // write to socket
    // if write finish stop write, start read.
    std::unique_lock<std::mutex> ul(io->buffers_p_mutex);
    if (!io->buffers_p.empty()) {
      auto p = io->buffers_p.front();
      ul.unlock();
      // change read io's isConnect
      YILOG_TRACE ("send data type {}", p->datatype());
      if (unlikely(p->datatype() == ChatType::clientconnectres)) {
        YILOG_TRACE ("set isConnect true");
        io_sp->isConnect = true;
      }
      // send
      if (p->socket_write(io->io.fd)) {
        ul.lock();
        io->buffers_p.pop();
        ul.unlock();
      }
    }else {
      ev_io_stop(loop, ww);
    }
  }else {
    YILOG_TRACE ("node released");
  }
}





