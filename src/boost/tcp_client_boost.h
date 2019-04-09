#ifndef TCP_CLIENT_BOOST_H_
#define TCP_CLIENT_BOOST_H_

#include "tcp_client.h"

#include <string>

#include <boost/asio.hpp>

class TcpClientBoost : public TcpClient
{
 public:
  TcpClientBoost(const std::string& address, const std::string& port);

  void start(const OnConnected& on_connected, const OnError& on_error) override;

 private:
  boost::asio::io_service m_io_service;
  boost::asio::ip::tcp::socket m_socket;
  boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator;
};

#endif  // TCP_CLIENT_BOOST_H_
