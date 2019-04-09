#ifndef TCP_SERVER_BOOST_H_
#define TCP_SERVER_BOOST_H_

#include "tcp_server.h"

#include <cstdint>

#include <boost/asio.hpp>

class TcpServerBoost : public TcpServer
{
 public:
  TcpServerBoost(std::uint16_t port, const OnAccept& on_accept);

  void start() override;

 private:
  OnAccept m_on_accept;
  boost::asio::io_service m_io_service;
  boost::asio::ip::tcp::acceptor m_acceptor;
  boost::asio::ip::tcp::socket m_socket;
};

#endif  // TCP_SERVER_BOOST_H_