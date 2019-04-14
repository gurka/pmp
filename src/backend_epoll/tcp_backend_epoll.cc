#include "tcp_backend.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tcp_client_epoll.h"
#include "tcp_server_epoll.h"
#include "logger.h"

std::unique_ptr<TcpBackend::Client> TcpBackend::create_client(const std::string& address,
                                                              const std::string& port,
                                                              const OnConnected& on_connected,
                                                              const OnError& on_error)
{
  LOG_ERROR("%s: not yet implemented", __func__);
  (void)address;
  (void)port;
  (void)on_connected;
  (void)on_error;
  return nullptr;
}

std::unique_ptr<TcpBackend::Server> TcpBackend::create_server(std::uint16_t port,
                                                              const OnAccept& on_accept)
{
  // Create socket
  auto socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0)
  {
    perror("socket");
    return nullptr;
  }

  // Bind socket
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  // Note: the reinterpret_cast breaks strict aliasing rules in C++, so
  //       this must be compiled with -fno-strict-aliasing in order to
  //       not invoke undefined behavior
  auto ret = bind(socket_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
  if (ret < 0)
  {
    perror("bind");
    return nullptr;
  }

  // Listen on socket
  static const auto backlog = 8;
  ret = listen(socket_fd, backlog);
  if (ret < 0)
  {
    perror("listen");
    return nullptr;
  }

  return std::make_unique<ServerEpoll>(socket_fd, on_accept);
}

void TcpBackend::run()
{
  LOG_ERROR("%s: not yet implemented", __func__);
}
