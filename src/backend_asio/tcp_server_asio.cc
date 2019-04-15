#include "tcp_server_asio.h"

#include "tcp_connection_asio.h"

namespace TcpBackend
{

using TCP = asio::ip::tcp;

ServerAsio::ServerAsio(asio::io_service* io_service,
                       std::uint16_t port,
                       const OnAccept& on_accept)
    : m_acceptor_v4(*io_service),
      m_socket_v4(*io_service),
      m_ongoing_v4(false),
      m_acceptor_v6(*io_service),
      m_socket_v6(*io_service),
      m_ongoing_v6(false),
      m_on_accept(on_accept)
{

  // Setup acceptors
  const auto endpoint_v4 = TCP::endpoint(TCP::v4(), port);
  m_acceptor_v4.open(endpoint_v4.protocol());
  m_acceptor_v4.set_option(asio::socket_base::reuse_address(true));
  m_acceptor_v4.bind(endpoint_v4);
  m_acceptor_v4.listen();

  // Note: option v6_only is needed because otherwise this socket
  // might try to listen on both IPv4 and IPv6, which will fail
  // because we already have an IPv4 socket listening
  const auto endpoint_v6 = TCP::endpoint(TCP::v6(), port);
  m_acceptor_v6.open(endpoint_v6.protocol());
  m_acceptor_v6.set_option(asio::socket_base::reuse_address(true));
  m_acceptor_v6.set_option(asio::ip::v6_only(true));
  m_acceptor_v6.bind(endpoint_v6);
  m_acceptor_v6.listen();
}

void ServerAsio::accept()
{
  if (!m_ongoing_v4)
  {
    m_acceptor_v4.async_accept(m_socket_v4, [this](const std::error_code& ec)
    {
      m_ongoing_v4 = false;

      if (!ec)
      {
        m_on_accept(std::make_unique<ConnectionAsio>(std::move(m_socket_v4)));
      }
      else
      {
        accept();
      }
    });

    m_ongoing_v4 = true;
  }

  if (!m_ongoing_v6)
  {
    m_acceptor_v6.async_accept(m_socket_v6, [this](const std::error_code& ec)
    {
      m_ongoing_v6 = false;

      if (!ec)
      {
        m_on_accept(std::make_unique<ConnectionAsio>(std::move(m_socket_v6)));
      }
      else
      {
        accept();
      }
    });

    m_ongoing_v6 = true;
  }
}

}
