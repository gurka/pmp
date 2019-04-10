#include "tcp_backend.h"

#include <boost/asio.hpp>

#include "tcp_connection_boost.h"
#include "tcp_client_boost.h"
#include "tcp_server_boost.h"

namespace TcpBackend
{

static boost::asio::io_service io_service;

std::unique_ptr<Client> create_client(const std::string& address,
                                      const std::string& port,
                                      const OnConnected& on_connected,
                                      const OnError& on_error)
{
  return std::make_unique<ClientBoost>(&io_service, address, port, on_connected, on_error);
}

std::unique_ptr<Server> create_server(std::uint16_t port, const OnAccept& on_accept)
{
  return std::make_unique<ServerBoost>(&io_service, port, on_accept);
}

void run()
{
  io_service.run();
}

}
