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
  asio::ip::tcp::acceptor m_acceptor_v4;
  asio::ip::tcp::socket   m_socket_v4;
  bool                    m_ongoing_v4;

  asio::ip::tcp::acceptor m_acceptor_v6;
  asio::ip::tcp::socket   m_socket_v6;
  bool                    m_ongoing_v6;

  OnAccept m_on_accept;
};

}

#endif  // TCP_SERVER_ASIO_H_
