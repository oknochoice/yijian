#ifndef IMNET_H_YIJIAN
#define IMNET_H_YIJIAN

#include "macro.h"

#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <list>

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

    YILOG_TRACE("func: {} ", __func__);

    boost::asio::async_read_until(*socket_.get(), request_, '\n',
        [this](const boost::system::error_code & ec,
              std::size_t bytes_transferred) {
          onRequestReceived(ec, bytes_transferred);
        });

  }
private:
  void onRequestReceived(const boost::system::error_code & ec,
      std::size_t bytes_transferred) {

    YILOG_TRACE("func: {} ", __func__);

    if (0 != ec) {
      YILOG_ERROR ("Error occured! Error code = {}. "
          "Message: {}", ec.value(), ec.message());
      onFinish();
      return;
    }

    response_ = ProcessRequest(request_);
    std::istream is(&request_);
    std::string s;
    is >> s;
    YILOG_INFO ("request : {}. \n"
      "response : {}", s, response_);
    
    boost::asio::async_write(*socket_.get(), 
        boost::asio::buffer(response_),
        [this](const boost::system::error_code & ec,
            std::size_t bytes_transferred) {
          onRequestSent(ec, bytes_transferred);
        });
  }
  void onRequestSent (const boost::system::error_code & ec, 
      std::size_t bytes_transferred) {

    YILOG_TRACE("func: {} ", __func__);

    if (0 != ec) {
      YILOG_ERROR ("Error occured! Error code {} . "
          "Message: {}", ec.value(), ec.message());
    }

    onFinish();
  }
  void onFinish () {

    YILOG_TRACE("func: {} ", __func__);

    
    delete this;
  }

  std::string ProcessRequest (boost::asio::streambuf & request) {

    YILOG_TRACE("func: {} ", __func__);

    int i = 0;
    while (i != 1'000'000) {
      i++;
    }
    std::string response = "Response\n";
    return response;
  }
public:
  std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
  std::string response_;
  boost::asio::streambuf request_;
};

class Acceptor {
public:
  Acceptor(boost::asio::io_service & ios, unsigned short port_num) 
    : ios_(ios), 
      acceptor_(ios_, 
        boost::asio::ip::tcp::endpoint(
          boost::asio::ip::address_v4::any(),
          port_num)),
      is_stopped_(false){

    YILOG_TRACE("func: {} ", __func__);
    
  }
  void Start() {

    YILOG_TRACE("func: {} ", __func__);

    acceptor_.listen();
    InitAccept();
  }
  void Stop() {

    YILOG_TRACE("func: {} ", __func__);
    
    is_stopped_.store(true);
  }
private:
  void InitAccept() {

    YILOG_TRACE("func: {} ", __func__);

    std::shared_ptr<boost::asio::ip::tcp::socket> 
      sock(new boost::asio::ip::tcp::socket(ios_));
    acceptor_.async_accept(*sock.get(),
      [this, sock](
        const boost::system::error_code & error) {
        onAccept(error, sock);
      });
  }
  void onAccept(const boost::system::error_code & ec,
          std::shared_ptr<boost::asio::ip::tcp::socket> sock) {

    YILOG_TRACE("func: {} ", __func__);

    if ( 0 == ec) {
      (new Connection(sock))->Start_work();
    }else {
      YILOG_ERROR("Error occured! Error code = {} .Message: {}", ec.value(), ec.message());
    }
    if (!is_stopped_.load()) {
      InitAccept();
    }else {
      acceptor_.close();
    }
  }
private:
  boost::asio::io_service & ios_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::atomic<bool> is_stopped_;
  std::list<std::shared_ptr<Connection>> list_;
};

class Server {
public:
  Server() {

    YILOG_TRACE("func: {} ", __func__);

    /*
    boost::asio::signal_set signal_set(ios_);
    signal_set.add(SIGINT);
    signal_set.add(SIGTERM);
    signal_set.add(SIGQUIT);
    signal_set.async_wait(boost::bind(&Server::Stop, this));
    */

  }

  void Start (unsigned short port_num) {

    YILOG_TRACE("func: {} ", __func__);

    acc_.reset(new Acceptor(ios_, port_num));
    acc_->Start();

    ios_.run();
  }
  void Stop() {

    YILOG_TRACE("func: {} ", __func__);

    acc_->Stop();
    ios_.stop();
  }
private:
private:
  boost::asio::io_service ios_;
  std::unique_ptr<Acceptor> acc_;
};


#endif
