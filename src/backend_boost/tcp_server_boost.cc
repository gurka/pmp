#include "tcp_server_boost.h"

#include "tcp_connection_boost.h"

namespace TcpBackend
{

using TCP = boost::asio::ip::tcp;

ServerBoost::ServerBoost(boost::asio::io_service* io_service,
                         std::uint16_t port,
                         const OnAccept& on_accept)
    : m_acceptor(*io_service, TCP::endpoint(TCP::v4(), port)),
      m_socket(*io_service),
      m_on_accept(on_accept)
{
  async_accept();
}

void ServerBoost::async_accept()
{
  m_acceptor.async_accept(m_socket, [this](const boost::system::error_code& ec)
  {
    if (!ec)
    {
      m_on_accept(std::make_unique<ConnectionBoost>(std::move(m_socket)));
    }

    async_accept();
  });
}

}
