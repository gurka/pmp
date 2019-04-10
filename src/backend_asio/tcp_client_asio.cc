#include "tcp_client_asio.h"

#include <memory>

#include "tcp_connection_asio.h"

namespace TcpBackend
{

using TCP = asio::ip::tcp;

ClientAsio::ClientAsio(asio::io_service* io_service,
                         const std::string& address,
                         const std::string& port,
                         const OnConnected& on_connected,
                         const OnError& on_error)
    : m_socket(*io_service),
      m_endpoint_iterator(TCP::resolver(*io_service).resolve({ address, port })),
      m_on_connected(on_connected),
      m_on_error(on_error)
{
  asio::async_connect(m_socket,
                      m_endpoint_iterator,
                      [this](const std::error_code& ec, TCP::resolver::iterator)
                      {
                        if (ec)
                        {
                          m_on_error(ec.message());
                        }
                        else
                        {
                          m_on_connected(std::make_unique<ConnectionAsio>(std::move(m_socket)));
                        }
                      });
}

}
