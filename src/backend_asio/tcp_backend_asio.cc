#include "tcp_backend.h"

#include <asio.hpp>

#include "tcp_connection_asio.h"
#include "tcp_client_asio.h"
#include "tcp_server_asio.h"

namespace TcpBackend
{

static asio::io_service io_service;

std::unique_ptr<Client> create_client(const std::string& address,
                                      const std::string& port,
                                      const OnConnected& on_connected,
                                      const OnError& on_error)
{
  return std::make_unique<ClientAsio>(&io_service, address, port, on_connected, on_error);
}

std::unique_ptr<Server> create_server(std::uint16_t port, const OnAccept& on_accept)
{
  return std::make_unique<ServerAsio>(&io_service, port, on_accept);
}

void run()
{
  io_service.run();
}

}
