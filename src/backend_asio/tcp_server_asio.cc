#include "tcp_server_asio.h"

#include "tcp_connection_asio.h"

namespace TcpBackend
{

using TCP = asio::ip::tcp;

ServerAsio::ServerAsio(asio::io_service* io_service,
                         std::uint16_t port,
                         const OnAccept& on_accept)
    : m_acceptor(*io_service, TCP::endpoint(TCP::v4(), port)),
      m_socket(*io_service),
      m_on_accept(on_accept)
{
  async_accept();
}

void ServerAsio::async_accept()
{
  m_acceptor.async_accept(m_socket, [this](const std::error_code& ec)
  {
    if (!ec)
    {
      m_on_accept(std::make_unique<ConnectionAsio>(std::move(m_socket)));
    }

    async_accept();
  });
}

}
