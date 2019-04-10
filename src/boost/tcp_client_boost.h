#ifndef TCP_CLIENT_BOOST_H_
#define TCP_CLIENT_BOOST_H_

#include "tcp_backend.h"

#include <string>

#include <boost/asio.hpp>

namespace TcpBackend
{

class ClientBoost : public Client
{
 public:
  ClientBoost(boost::asio::io_service* io_service,
              const std::string& address,
              const std::string& port,
              const OnConnected& on_connected,
              const OnError& on_error);

 private:
  boost::asio::ip::tcp::socket m_socket;
  boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator;
  OnConnected m_on_connected;
  OnError m_on_error;
};

}

#endif  // TCP_CLIENT_BOOST_H_
