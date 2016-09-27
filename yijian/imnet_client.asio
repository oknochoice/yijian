#ifndef IMNET_CLIENT_H_YIJIAN
#define IMNET_CLIENT_H_YIJIAN

#include "macro.h"

#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace boost;

class SyncTCPClient {
public:
  SyncTCPClient(const std::string & raw_ip_address, 
      unsigned short port_num)
    : m_ep(asio::ip::address::from_string(raw_ip_address),
        port_num),
      m_sock(m_ios) {
    YILOG_TRACE("func: {} ", __func__);
    m_sock.open(m_ep.protocol());
  }
  void connect() {
    YILOG_TRACE("func: {} ", __func__);
    m_sock.connect(m_ep);
  }
  void close() {
    YILOG_TRACE("func: {} ", __func__);
    m_sock.shutdown(asio::ip::tcp::socket::shutdown_both);
    m_sock.close();
  }
  std::string emulateLongComputionOp(
      unsigned int duration_sec) {
    YILOG_TRACE("func: {} ", __func__);
    std::string request = "emulate_long_comp_op "
      + std::to_string(duration_sec)
      + "\n";
    sendRequest(request);
    return receiveResponse();
  }
private:
  void sendRequest(const std::string & request) {
    YILOG_TRACE("func: {} ", __func__);
    asio::write(m_sock, asio::buffer(request));
  }
  std::string receiveResponse() {
    YILOG_TRACE("func: {} ", __func__);
    asio::streambuf buf;
    asio::read_until(m_sock, buf, '\n');
    std::istream input(&buf);
    std::string response;
    std::getline(input, response);
    return response;
  }
private:
  asio::io_service m_ios;
  asio::ip::tcp::endpoint m_ep;
  asio::ip::tcp::socket m_sock;
};

#endif
