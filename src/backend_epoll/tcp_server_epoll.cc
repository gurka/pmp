#include "tcp_server_epoll.h"

#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tcp_connection_epoll.h"

// static creator
std::unique_ptr<TcpServer> TcpServer::create(std::uint16_t port, const OnAccept& on_accept)
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

  return std::make_unique<TcpServerEpoll>(socket_fd, on_accept);
}

TcpServerEpoll::TcpServerEpoll(int socket_fd, const OnAccept& on_accept)
    : m_socket_fd(socket_fd),
      m_on_accept(on_accept)
{
}

void TcpServerEpoll::start()
{
  while (true)
  {
    // Ignore connection address
    auto socket_fd = accept(m_socket_fd, nullptr, nullptr);
    if (socket_fd < 0)
    {
      perror("accept");
      continue;
    }

    m_on_accept(std::make_unique<TcpConnectionEpoll>(socket_fd));
  }
}
