#ifndef TCP_SERVER_ASIO_H_
#define TCP_SERVER_ASIO_H_

#include "tcp_backend.h"

#include <cstdint>

#include <asio.hpp>

namespace TcpBackend
{

class ServerAsio : public Server
{
 public:
  ServerAsio(asio::io_service* io_service,
             std::uint16_t port,
             const OnAccept& on_accept);

  void accept() override;

 private:
  asio::ip::tcp::acceptor m_acceptor;
  asio::ip::tcp::socket m_socket;
  OnAccept m_on_accept;
};

}

#endif  // TCP_SERVER_ASIO_H_
