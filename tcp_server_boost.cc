#include "tcp_server_boost.h"

#include <cstdio>

#include "tcp_connection_boost.h"

// static creator
std::unique_ptr<TcpServer> TcpServer::create(std::uint16_t port, const OnAccept& on_accept)
{
  return std::make_unique<TcpServerBoost>(port, on_accept);
}

using TCP = boost::asio::ip::tcp;

TcpServerBoost::TcpServerBoost(std::uint16_t port, const OnAccept& on_accept)
    : m_on_accept(on_accept),
      m_io_service(),
      m_acceptor(m_io_service, TCP::endpoint(TCP::v4(), port)),
      m_socket(m_io_service)
{
}

void TcpServerBoost::start()
{
  m_acceptor.async_accept(m_socket, [this](const boost::system::error_code& ec)
  {
    if (!ec)
    {
      m_on_accept(std::make_unique<TcpConnectionBoost>(std::move(m_socket)));
    }

    start();
  });

  m_io_service.run();
}
