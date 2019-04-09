#include "tcp_client_boost.h"

#include <string>
#include <memory>

#include "tcp_connection_boost.h"

// static creator
std::unique_ptr<TcpClient> TcpClient::create(const std::string& address, const std::string& port)
{
  return std::make_unique<TcpClientBoost>(address, port);
}

using TCP = boost::asio::ip::tcp;

TcpClientBoost::TcpClientBoost(const std::string& address, const std::string& port)
    : m_io_service(),
      m_socket(m_io_service),
      m_endpoint_iterator(TCP::resolver(m_io_service).resolve({ address, port }))
{
}

void TcpClientBoost::start(const OnConnected& on_connected, const OnError& on_error)
{
  boost::asio::async_connect(m_socket,
                             m_endpoint_iterator,
                             [this, on_connected, on_error](const boost::system::error_code& ec, TCP::resolver::iterator)
                             {
                               if (ec)
                               {
                                 on_error(ec.message());
                               }
                               else
                               {
                                 on_connected(std::make_unique<TcpConnectionBoost>(std::move(m_socket)));
                               }
                             });

  m_io_service.run();
}
