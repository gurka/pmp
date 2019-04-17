#include "tcp_backend.h"

#include <asio.hpp>

#include "tcp_connection_asio.h"
#include "tcp_server_asio.h"

namespace TcpBackend
{

static asio::io_service io_service;

bool connect(const std::string& address,
             const std::string& port,
             const OnConnected& on_connected,
             const OnError& on_error)
{
  // Create the socket in a unique_ptr and then capture it by move into the lambda
  // That way it will survive until the async call has finished
  auto socket = std::make_unique<asio::ip::tcp::socket>(io_service);
  auto* socket_ptr = socket.get();
  asio::async_connect(*socket_ptr,
                      asio::ip::tcp::resolver(io_service).resolve({ address, port }),
                      [socket = std::move(socket), address, port, on_connected, on_error]
                      (const std::error_code& ec, const asio::ip::tcp::endpoint&)
                      {
                        if (ec)
                        {
                          on_error(ec.message());
                        }
                        else
                        {
                          on_connected(std::make_unique<ConnectionAsio>(std::move(*socket)),
                                       address,
                                       port);
                        }
                      });
  return true;
}

std::unique_ptr<Server> create_server(std::uint16_t port, const OnAccept& on_accept)
{
  return std::make_unique<ServerAsio>(&io_service, port, on_accept);
}

void run()
{
  io_service.run();
}

void stop()
{
  io_service.stop();
}

}
