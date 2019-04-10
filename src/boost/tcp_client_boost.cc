#include "tcp_client_boost.h"

#include <memory>

#include "tcp_connection_boost.h"

namespace TcpBackend
{

using TCP = boost::asio::ip::tcp;

ClientBoost::ClientBoost(boost::asio::io_service* io_service,
                         const std::string& address,
                         const std::string& port,
                         const OnConnected& on_connected,
                         const OnError& on_error)
    : m_socket(*io_service),
      m_endpoint_iterator(TCP::resolver(*io_service).resolve({ address, port })),
      m_on_connected(on_connected),
      m_on_error(on_error)
{
  boost::asio::async_connect(m_socket,
                             m_endpoint_iterator,
                             [this](const boost::system::error_code& ec, TCP::resolver::iterator)
                             {
                               if (ec)
                               {
                                 m_on_error(ec.message());
                               }
                               else
                               {
                                 m_on_connected(std::make_unique<ConnectionBoost>(std::move(m_socket)));
                               }
                             });
}

}
