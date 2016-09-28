#ifndef IMNET_H_YIJIAN
#define IMNET_H_YIJIAN

#include "macro.h"

#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <list>
#include <ctime>
#include <ev.h>

struct Imp_connection {
  ev_io io_;
  time_t last_msg_time_;
};

class Connection {
public:
  Connection(std::shared_ptr<boost::asio::ip::tcp::socket> & s)
    : socket_(s) {

    YILOG_TRACE("func: {} ", __func__);

  }
  ~Connection() {
    
    YILOG_TRACE("func: {} ", __func__);

    socket_->close();

  }
  void Start_work() {

    YILOG_TRACE("Connection func: {} ", __func__);

  }
private:
  void onRequestReceived(const boost::system::error_code & ec,
      std::size_t bytes_transferred) {

    YILOG_TRACE("Connection func: {} ", __func__);

  }
  void onRequestSent (const boost::system::error_code & ec, 
      std::size_t bytes_transferred) {

    YILOG_TRACE("Connection func: {} ", __func__);

    onFinish();
  }
  void onFinish () {

    YILOG_TRACE("func: {} ", __func__);
    
    delete this;
  }

public:
  time_t last_msg_time_;
  std::string response_;
  std::list<std::shared_ptr<Connection>>::iterator iterator;

};

class Server {
public:
  Server() {

    YILOG_TRACE("func: {} ", __func__);

  }

  void Start (unsigned short port_num) {

    YILOG_TRACE("Server func: {} ", __func__);

  }
private:
};


#endif
