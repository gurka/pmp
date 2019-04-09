#ifndef TCP_CONNECTION_BOOST_H_
#define TCP_CONNECTION_BOOST_H_

#include "tcp_connection.h"

#include <boost/asio.hpp>

class TcpConnectionBoost : public TcpConnection
{
 public:
  TcpConnectionBoost(boost::asio::ip::tcp::socket socket);

  void close() override;

 private:
  boost::asio::ip::tcp::socket m_socket;
};

#endif  // TCP_CONNECTION_BOOST_H_
