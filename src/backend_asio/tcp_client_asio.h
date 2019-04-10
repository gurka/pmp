#ifndef TCP_CLIENT_ASIO_H_
#define TCP_CLIENT_ASIO_H_

#include "tcp_backend.h"

#include <string>

#include <asio.hpp>

namespace TcpBackend
{

class ClientAsio : public Client
{
 public:
  ClientAsio(asio::io_service* io_service,
              const std::string& address,
              const std::string& port,
              const OnConnected& on_connected,
              const OnError& on_error);

 private:
  asio::ip::tcp::socket m_socket;
  asio::ip::tcp::resolver::iterator m_endpoint_iterator;
  OnConnected m_on_connected;
  OnError m_on_error;
};

}

#endif  // TCP_CLIENT_ASIO_H_
