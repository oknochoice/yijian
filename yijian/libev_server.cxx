#include "libev_server.h"
#include <map>
#include <functional>
#include <mutex>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <buffer.h>
#include <list>
#include <tuple>

#include "server_msg_typemap.h"

#define PINGPONGTIME 1200
#define PINGPONGTIMEOUT 1200

#define QUICKREMOVETIME 10
#define QUICKREMOVETIMEOUT 10
/*
 *
 * priate declare
 *
 * */

static struct ev_loop * loop();
static struct ev_io * accept_watcher();
static struct ev_timer * timer_watcher();
static struct ev_timer * quicktimer_watcher();
static struct Write_Asyn * write_asyn_watcher();
static struct ev_async * reread_asyn_watcher();
static yijian::noti_threads * noti_threads();

static void
sigint_cb (struct ev_loop * loop, ev_signal * w, int revents);
// ev_async watcher priority si EV_MAXPRI
static void
start_write_callback (struct ev_loop * loop, ev_async * w, int revents);
static void
reread_callback (struct ev_loop * loop, ev_async * w, int revents);
static void
socket_accept_callback (struct ev_loop * loop, ev_io * rw, int revents);
static void
connection_read_callback (struct ev_loop * loop, ev_io * rw, int revents);
static void
connection_write_callback (struct ev_loop * loop, ev_io * ww, int revents);

static bool isStopThreadFunc_ = false;

void uuidnode_delete(const std::string & uuid);
/*
 * ping list
 *
 * */

uint64_t ping_time() {
  YILOG_TRACE ("func: {}. ", __func__);
  auto now = std::chrono::high_resolution_clock::now();
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
      now.time_since_epoch());
  YILOG_TRACE ("func: {}. time:{}", __func__, seconds.count());
  return seconds.count();
}
static Imp_list pinglist_;
bool ping_erase(Read_IO_SP sp) {
  YILOG_TRACE ("func: {}. ", __func__);
  if(likely(sp->iter_sp != nullptr)) {
    pinglist_.erase(*sp->iter_sp);
    YILOG_TRACE ("func: {}. pinglist count: {}.", 
        __func__, pinglist_.size());
    return true;
  }else {
    YILOG_TRACE ("func: {}. pinglist count: {}.", 
        __func__, pinglist_.size());
    return false;
  }
}
void ping_append(Read_IO_SP sp) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto iter = pinglist_.insert(pinglist_.end(), sp);
  sp->iter_sp.reset(new Iter(iter));
  sp->ping_time = ping_time();
    YILOG_TRACE ("func: {}. pinglist count: {}.", 
        __func__, pinglist_.size());
}
void ping_move2back(Read_IO_SP sp) {
  YILOG_TRACE ("func: {}. ", __func__);
  if(ping_erase(sp)) {
    ping_append(sp);
    YILOG_TRACE ("func: {}. pinglist count: {}.", 
        __func__, pinglist_.size());
  }
}
void ping_foreach(std::function<void(Read_IO_SP, bool * )> && func) {
  YILOG_TRACE ("func: {}. ", __func__);
  bool isStop = false;
  for (auto io: pinglist_) {
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
        if (now - sp->ping_time > PINGPONGTIMEOUT) {
          YILOG_INFO ("erase uuid:{} from pingtime.", sp->uuid);
          uuidnode_delete(sp->uuid);
          ping_erase(sp);
          *isStop = false;
        }else {
          *isStop = true;
        }
      });
  ev_timer_set (w, PINGPONGTIME, 0.);

  // stop server
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
      free   (quicktimer_watcher());
      free   (accept_watcher());
      delete write_asyn_watcher();
      delete reread_asyn_watcher();
      ev_loop_destroy(loop);
      exit(0);
    }
  }
  ev_timer_start (loop, w);
}
/*
 * quick remove node
 *
 * */
struct QuickRemove {
  Imp_list first_;
  Imp_list second_;
  uint_fast32_t index_ = 0;
};

QuickRemove quickRemove_;
void quickremove_append(Read_IO_SP sp)  {
  YILOG_TRACE ("func: {}. ", __func__);
  if (quickRemove_.index_ % 2 == 0) {
    quickRemove_.first_.push_back(sp);
  }else {
    quickRemove_.second_.push_back(sp);
  }
}

static void 
quickremove_callback(EV_P_ ev_timer *w, int revents) {
  YILOG_TRACE ("func: {}. ", __func__);
  ev_timer_stop(loop, w);
  uint64_t now = ping_time();
  ++quickRemove_.index_;
  if (quickRemove_.index_ % 2 == 0) {
    quickRemove_.first_.clear();
  }else {
    quickRemove_.second_.clear();
  }
  YILOG_TRACE ("func: {}. first_ count:{}, second_ count{}",
      __func__, quickRemove_.first_.size(),
      quickRemove_.second_.size());
  ev_timer_set (w, QUICKREMOVETIME, 0.);
  ev_timer_start (loop, w);
}
/*
 *
 * uuid key read io pointor value: map
 *
 *
 * */ 
static std::unordered_map<std::string, std::shared_ptr<Read_IO>> uuidnode_map_;

void uuidnode_put(const std::string & uuid, std::shared_ptr<Read_IO> io) {
  YILOG_TRACE ("func: {}. ", __func__);
  uuidnode_map_[uuid] = io;
  YILOG_TRACE ("func: {}. uuidnode map count: {}.", 
      __func__, uuidnode_map_.size());
}

void uuidnode_delete(const std::string & uuid) {
  YILOG_TRACE ("func: {}. ", __func__);
  uuidnode_map_.erase(uuid);
  YILOG_TRACE ("func: {}. uuidnode map count: {}.", 
      __func__, uuidnode_map_.size());
}

std::shared_ptr<Read_IO> uuidnode_get(const std::string & uuid) {
  YILOG_TRACE ("func: {}. ", __func__);
  YILOG_TRACE ("func: {}. uuidnode map count: {}.", 
      __func__, uuidnode_map_.size());
  return uuidnode_map_.at(uuid);
}

void mountBuffer2Device(const std::string & uuid, 
    std::function<void(std::shared_ptr<Read_IO>)> && func) {
  YILOG_TRACE ("func: {}. ", __func__);
  auto it = uuidnode_map_.find(uuid);
  if (likely(it != uuidnode_map_.end())) {
    YILOG_INFO ("func: {}. find", __func__);
    func(it->second);
  }else{
    YILOG_INFO ("func: {}. find", __func__);
  }
}
/*
 * need re start read vector
 *
 * */
static std::vector<std::shared_ptr<Read_IO>> reStartRead_vec_;
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
  auto iter = peer_servers_.io_list_->insert(peer_servers_.io_list_->end(), io);
  io->iter_sp.reset(new Iter(iter));
}

void peer_servers_erase(std::shared_ptr<Read_IO> io) {
  YILOG_TRACE ("func: {}. ", __func__);
  std::unique_lock<std::mutex> ul(peer_servers_.mutex_);
  if (!peer_servers_.io_list_.unique()) {
    peer_servers_.io_list_.reset(new 
        std::list<std::shared_ptr<Read_IO>>(*peer_servers_.io_list_));
  }
  if (likely(io->iter_sp != nullptr)) {
    peer_servers_.io_list_->erase(*io->iter_sp);
  }
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
  auto lloop = loop();
  // Connection_IO watcher for client
  std::shared_ptr<Read_IO> client_read_watcher(new Read_IO,
      [lloop](Read_IO * io){
        try {
          YILOG_INFO("close read io");
          ev_io_stop(lloop, &io->io);
          ev_io_stop(lloop, &io->writeio_sp->io);
          close(io->io.fd);
          delete io;
        }catch(...) {
          YILOG_ERROR("close read io error");
        }
      });
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

  ev_io_start (lloop, &client_read_watcher->io);

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

static struct ev_timer * quicktimer_watcher() {
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

static struct ev_async * reread_asyn_watcher() {
  YILOG_TRACE ("func: {}. ", __func__);

  // ev_async
  static ev_async * watcher = new ev_async();

  return watcher;
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

  // manager thread
  if (0 > open_thread_manager()) {
    perror("thread manager open Failed");
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
  auto lloop = loop();
  // signal
  ev_signal signal_int_watcher;
  ev_signal_init (&signal_int_watcher, sigint_cb, SIGINT);
  ev_signal_start (lloop, &signal_int_watcher);

  ev_signal signal_user1_watcher;
  ev_signal_init(&signal_user1_watcher, sigusr1_cb, SIGUSR1);
  ev_signal_start(lloop, &signal_user1_watcher);

  // ev_async
  struct ev_async * async_io = &write_asyn_watcher()->as;
  ev_async_init(async_io, start_write_callback);
  ev_set_priority(async_io, EV_MAXPRI);
  ev_async_start(lloop, async_io);

  // ev_async re read
  struct ev_async * reread_io = reread_asyn_watcher();
  ev_async_init(reread_io, reread_callback);
  ev_set_priority(reread_io, EV_MAXPRI);
  ev_async_start(lloop, reread_io);
  
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
  ev_io_start(lloop, accept_io);


  //timer ping
  struct ev_timer * timer = timer_watcher();
  ev_timer_init (timer, pingtime_callback, PINGPONGTIME, 0.);
  ev_timer_start (lloop, timer);

  //timer quick remove
  struct ev_timer * quick_timer = quicktimer_watcher();
  ev_timer_init (quick_timer, quickremove_callback, QUICKREMOVETIME, 0.);
  ev_timer_start (lloop, quick_timer);

  ev_run (lloop, 0);

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
        //ev_io_start(loop, &io->io);
        ev_io_start(loop, &io->writeio_sp->io);
      });

}

static void
reread_callback (struct ev_loop * loop,  ev_async * r, int revents) {
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
  std::shared_ptr<Read_IO> client_read_watcher(new Read_IO,
      [loop](Read_IO * io){
        try {
          YILOG_INFO("close read io");
          ev_io_stop(loop, &io->io);
          ev_io_stop(loop, &io->writeio_sp->io);
          close(io->io.fd);
          delete io;
        }catch(...) {
          YILOG_ERROR("close read io error");
        }
      });
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
  quickremove_append(client_read_watcher);
}

static void 
connection_read_callback (struct ev_loop * loop, 
    ev_io * rw, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  try {
    //ev_io_stop(loop, rw);

    // converse to usable io
    Read_IO * io = reinterpret_cast<Read_IO*>(rw);
    auto io_sp = io->self.lock();
    if(io_sp) {

      // read to buffer 
      // if read complete stop watch 
      if (io->buffer_sp->socket_read(io->io.fd)) {
        // update ping time
        YILOG_TRACE ("func: {}. update ping time", __func__);
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
          YILOG_INFO ("pingtime:{}, isConnect:{}, sessionid:{}, "
              "userid:{}, uuid:{}", 
              io_sp->ping_time, 
              io_sp->isConnect, 
              io_sp->sessionid, 
              io_sp->userid, 
              io_sp->uuid);
          /*
          std::tuple<std::shared_ptr<Read_IO>, 
            std::shared_ptr<yijian::buffer>,
            uint16_t> t(io_sp, sp, sessionid);
            */
          YILOG_DEBUG("buffer sp reference count {}",
              sp.use_count());
          noti_threads()->sentWork(
              [loop, watcher, io_sp, sp, sessionid](){
                YILOG_TRACE ("dispatch message");
                YILOG_DEBUG("buffer sp reference count {}",
                    sp.use_count());
                dispatch(io_sp, sp, sessionid);
                ev_async_send(loop, watcher);
              });
        }
        
        YILOG_DEBUG("buffer sp reference count {}", 
            io->buffer_sp.use_count());
        io->buffer_sp.reset(new yijian::buffer());
      }else{
        YILOG_TRACE ("read is not complete message");
        YILOG_ERROR ("read is not complete message");
        //ev_io_start(loop, rw);
      }
    }else {
      YILOG_TRACE ("node is released");
      YILOG_ERROR ("node is released");
    }

  }catch (std::system_error & e) {
    YILOG_ERROR ("errno: {}, errmsg: {}.",
        e.code().value(), e.what());
    if (e.code().value() == 20002 ||
        e.code().value() == 20003) {
      // close node
      ev_io_stop(loop, rw);
      Read_IO * io = reinterpret_cast<Read_IO*>(rw);
      auto io_sp = io->self.lock();
      YILOG_INFO ("read io use count:{}", io_sp.use_count());
      ping_erase(io_sp);
      YILOG_INFO ("read io use count:{}", io_sp.use_count());
      uuidnode_delete(io_sp->uuid);
      YILOG_INFO ("read io use count:{}", io_sp.use_count());
    }
  }
}

static void 
connection_write_callback (struct ev_loop * loop, 
    ev_io * ww, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  Write_IO * io = reinterpret_cast<Write_IO*>(ww);
  auto read_io_sp = io->readio_sp.lock();
  if (read_io_sp) {
    // write to socket
    // if write finish stop write, start read.
    std::unique_lock<std::mutex> ul(io->buffers_p_mutex);
    if (!io->buffers_p.empty()) {
      auto p = io->buffers_p.front();
      ul.unlock();
      YILOG_INFO ("send sessionid: {}, data type: {}, "
          "userid:{}, uuid:{}. \n"
          "write buffers count {}.", 
          p->session_id(), p->datatype(), 
          read_io_sp->userid, read_io_sp->uuid,
          io->buffers_p.size());
      
      
      // update pingnode
      ping_move2back(read_io_sp);
      // change read io's isConnect
      if (unlikely(p->datatype() == ChatType::clientconnectres)) {
        YILOG_INFO ("clientconnectres set isConnect true");
        read_io_sp->isConnect = true;
        if (likely(!read_io_sp->uuid.empty())) {
          // remove old node from pinglist;
          try {
            auto lsp = uuidnode_get(read_io_sp->uuid);
            if (lsp.get() != read_io_sp.get()) {
              YILOG_TRACE ("erase old Read_IO from pinglist");
              ping_erase(lsp);
            }
          }catch(std::out_of_range & e) {
            YILOG_TRACE ("node not in pinglist");
          }
          // append new
          ping_append(read_io_sp);
          // uuid node map
          uuidnode_put(read_io_sp->uuid, read_io_sp);
        }else {
          YILOG_ERROR ("uuid not find in client connect res, connect write callback");
        }
      }
      if (unlikely(p->datatype() == ChatType::clientdisconnectres)) {
        YILOG_INFO ("clientdisconnectres delete io map & ping list");
        if (likely(!read_io_sp->uuid.empty())) {
          uuidnode_delete(read_io_sp->uuid);
          ping_erase(read_io_sp);
        }else {
          YILOG_ERROR ("uuid not find in client disconnect res, connect write callback");
        }
      }
      YILOG_INFO ("pinglist count: {}.", pinglist_.size());
      YILOG_INFO ("uuidnode map count: {}.", uuidnode_map_.size());
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
    ev_io_stop(loop, ww);
  }
}





