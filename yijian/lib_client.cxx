#include "lib_client.h"
#include <ev.h>
#include <utility>
#include <queue>
#include <condition_variable>

#include <openssl/err.h>
#include <netinet/tcp.h>

#include "typemap.h"

#define HOST_NAME "www.yijian.com"
#define HOST_PORT "5555"

struct Read_IO {
  // watcher
  ev_io io;
  // socket buffer
  Buffer_SP buffer_sp = std::make_shared<yijian::buffer>();
  uint16_t sessionid;
  // ssl
  SSL * ssl;
};

struct Write_IO {
  // watcher
  ev_io io;
  // socket buffer
  std::mutex buffers_p_mutex;
  std::queue<Buffer_SP> buffers_p;
  // ssl
  SSL * ssl;
};

static std::shared_ptr<Read_CB> sp_read_cb_;
static Read_IO * read_io_;
static Write_IO * write_io_;
static std::mutex ev_c_mutex_;
static bool ev_c_isWait_ = true;
static std::condition_variable ev_c_var_;

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

/*
 *
 * openssl
 *
 *
 * */
void init_openssl() {
  YILOG_TRACE ("func: {}. ", __func__);
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}
void cleanup_openssl() {
  YILOG_TRACE ("func: {}. ", __func__);
  EVP_cleanup();
}
SSL_CTX * create_sslctx() {
  YILOG_TRACE ("func: {}. ", __func__);
  const SSL_METHOD * method;
  SSL_CTX * ctx;

  method = TLSv1_2_client_method();

  ctx = SSL_CTX_new(method);
  if (!ctx) {
    ERR_print_errors_fp(stderr);
    YILOG_ERROR("Unable to create SSL context");
    throw std::system_error(std::error_code(60004, std::generic_category()),
        "Unable to create SSL context");
  }
  return ctx;
}
SSL_CTX * sslctx() {
  YILOG_TRACE ("func: {}. ", __func__);
  static SSL_CTX * ctx = create_sslctx();
  return ctx;
}
int verify_callback(int preverify, X509_STORE_CTX * x509_ctx) {
  YILOG_TRACE ("func: {}. ", __func__);
  int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
  int err = X509_STORE_CTX_get_error(x509_ctx);

  X509 * cert = X509_STORE_CTX_get_current_cert(x509_ctx);

  if (0 == depth) {

  }
  return preverify;
}
void configure_sslctx(SSL_CTX * ctx, SSL ** ssl) {
  YILOG_TRACE ("func: {}. ", __func__);

  //SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, 0);
  SSL_CTX_set_verify_depth(ctx, 4);

  const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
  SSL_CTX_set_options(ctx, flags);

  if ( 1 != SSL_CTX_load_verify_locations(ctx, "root-ca.crt", NULL)) {
    ERR_print_errors_fp(stderr);
    throw std::system_error(std::error_code(60005, std::generic_category()),
        "could load location ca failure");
  }

  BIO * web = BIO_new_ssl_connect(ctx);
  if ( 1 != BIO_set_conn_hostname(web, HOST_NAME ":" HOST_PORT)) {
    ERR_print_errors_fp(stderr);
    throw std::system_error(std::error_code(60006, std::generic_category()),
        "new web failure");
  }

  BIO_get_ssl(web, ssl);
  if (!(NULL != *ssl)) {
    ERR_print_errors_fp(stderr);
    throw std::system_error(std::error_code(60007, std::generic_category()),
        "init ssl failure");
  }
  const char* const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
  if (SSL_CTX_set_cipher_list(ctx, PREFERRED_CIPHERS) != 1) {
    ERR_print_errors_fp(stderr);
    throw std::system_error(std::error_code(60003, std::generic_category()),
        "could not set cipher list");
  }

  if (1 != SSL_set_tlsext_host_name(*ssl, HOST_NAME)) {
    ERR_print_errors_fp(stderr);
  }

  BIO * out = BIO_new_fp(stdout, BIO_NOCLOSE);

  if (1 != BIO_do_connect(web)) {
    ERR_print_errors_fp(stderr);
    throw std::system_error(std::error_code(60008, std::generic_category()),
        "BIO_do_connect failure");
  }

  if (1 != BIO_do_handshake(web)) {
    ERR_print_errors_fp(stderr);
    throw std::system_error(std::error_code(60009, std::generic_category()),
        "BIO_do_handshake failure");
  }



  SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
  SSL_CTX_set_mode(ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
}

void start_write_callback (struct ev_loop * loop,  ev_async * r, int revents) {
  
  YILOG_TRACE ("func: {}. ", __func__);
  //ev_io_start(loop, &read_io_->io);
  ev_io_start(loop, &write_io_->io);

}

void connection_read_callback (struct ev_loop * loop, 
    ev_io * rw, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  try {
  //ev_io_stop(loop, rw);
  // converse to usable io
  Read_IO * io = reinterpret_cast<Read_IO*>(rw);

  // read to buffer 
  // if read complete stop watch && update ping
  
  if (io->buffer_sp->socket_read(io->ssl)) {
    YILOG_TRACE ("func: {}. receive datatype {}.", 
        __func__, io->buffer_sp->datatype());
    if (unlikely(io->buffer_sp->datatype() == chatType::clientconnectres)) {
      YILOG_INFO ("func: {}. set sessionid", __func__);
      read_io_->sessionid = io->buffer_sp->session_id();
    }
    (*sp_read_cb_)(io->buffer_sp);
    io->buffer_sp.reset(new yijian::buffer());
  }

  }catch (std::system_error & e) {
    YILOG_ERROR ("errno: {}, errmsg: {}.",
        e.code().value(), e.what());
    if (e.code().value() == 20002 ||
        e.code().value() == 20003 ||
        e.code().value() == 20005 ||
        e.code().value() == 20006 ||
        e.code().value() == 20007 ||
        e.code().value() == 20008) {
      // close node
    }
    throw std::system_error(std::error_code(60000, std::generic_category()),
        "need close client and restart");
  }

}

void connection_write_callback (struct ev_loop * loop, 
    ev_io * ww, int revents) {

  YILOG_TRACE ("func: {}. ", __func__);

  // converse to usable io
  Write_IO * io = reinterpret_cast<Write_IO*>(ww);

  // write to socket
  // if write finish stop write, start read.
  std::unique_lock<std::mutex> ul(io->buffers_p_mutex);
  if (!io->buffers_p.empty()) {
    auto p = io->buffers_p.front();
    ul.unlock();
    if (p->socket_write(io->ssl)) {
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

static void init_io() {
  YILOG_TRACE ("func: {}. ", __func__);

  // ev_async
  struct ev_async * async_io = write_asyn_watcher();
  ev_async_init(async_io, start_write_callback);
  ev_set_priority(async_io, EV_MAXPRI);
  ev_async_start(loop(), async_io);
  YILOG_TRACE("ev_async init Success");

  // Connection_IO watcher for client
  read_io_ = new Read_IO();
  write_io_ = new Write_IO();

  if (NULL == read_io_ || 
      NULL == write_io_) {
    throw std::system_error(std::error_code(60002, std::generic_category()),
        "new client watcher error");
  }

  // ssl connect
  init_openssl();
  SSL_CTX * ctx = sslctx();
  SSL * ssl = NULL;
  configure_sslctx(ctx, &ssl);
  read_io_->ssl = ssl;
  write_io_->ssl = ssl;

  int sfd = SSL_get_fd(ssl);

  //int flag = 1;
  //if ( 0 > setsockopt(sfd, IPPROTO_TCP, 
        //TCP_NODELAY, (char*)&flag, sizeof(int))) {
    //YILOG_ERROR ("set TCP_NODELAY failure");
  //}

  YILOG_TRACE("Connected Success.");
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
    init_io();
    sp_read_cb_.reset(new Read_CB(std::forward<Read_CB>(read_cb)));
    std::unique_lock<std::mutex> ul(ev_c_mutex_);
    ev_c_isWait_ = false;
    ev_c_var_.notify_one();
    ul.unlock();
    ev_run(loop(), 0);
    YILOG_TRACE("exit thread");
  });
  t.detach();
  std::unique_lock<std::mutex> cul(ev_c_mutex_);
  ev_c_var_.wait(cul, [&](){
        return !ev_c_isWait_;
      });

}

void client_send(Buffer_SP sp_buffer,
    uint16_t * sessionid) {
  YILOG_TRACE ("func: {}. ", __func__);
  
  auto sid = read_io_->sessionid++;
  YILOG_INFO("send sessionid {}", sid);
  if (nullptr != sessionid) {
    *sessionid = sid;
  }
  sp_buffer->set_sessionid(sid);
  std::unique_lock<std::mutex> ul(write_io_->buffers_p_mutex);
  write_io_->buffers_p.push(sp_buffer);
  auto watcher = write_asyn_watcher();
  
  ev_async_send(loop(), watcher);

}

void clear_client() {
  YILOG_TRACE ("func: {}. ", __func__);
  SSL_free(read_io_->ssl);
  SSL_CTX_free(sslctx());
  cleanup_openssl();
  close(read_io_->io.fd);
  ev_io_stop(loop(), &read_io_->io);
  ev_io_stop(loop(), &write_io_->io);
  ev_async_stop(loop(), write_asyn_watcher());
  ev_loop_destroy(loop());
  delete read_io_;
  delete write_io_;
  free(write_asyn_watcher());
}



