#include "tcp_server_boost.h"

#include <cstdio>

#include "tcp_connection_boost.h"

// static creator
std::unique_ptr<TcpServer> TcpServer::create(std::uint16_t port)
{
  return std::make_unique<TcpServerBoost>(port);
}

using TCP = boost::asio::ip::tcp;

TcpServerBoost::TcpServerBoost(std::uint16_t port)
    : m_io_service(),
      m_acceptor(m_io_service, TCP::endpoint(TCP::v4(), port)),
      m_socket(m_io_service)
{
}

void TcpServerBoost::start(const OnAccept& on_accept)
{
  m_acceptor.async_accept(m_socket, [this, on_accept](const boost::system::error_code& ec)
  {
    if (!ec)
    {
      on_accept(std::make_unique<TcpConnectionBoost>(std::move(m_socket)));
    }

    start(on_accept);
  });

  m_io_service.run();
}
