#include "tcp_server_epoll.h"

#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>

#include "tcp_connection_epoll.h"

namespace TcpBackend
{

ServerEpoll::ServerEpoll(int socket_fd, const OnAccept& on_accept)
    : m_socket_fd(socket_fd),
      m_on_accept(on_accept)
{
}

void ServerEpoll::accept()
{
#if 0
  // Ignore connection address
  auto socket_fd = ::accept(m_socket_fd, nullptr, nullptr);
  if (socket_fd < 0)
  {
    perror("accept");
    continue;
  }

  m_on_accept(std::make_unique<ConnectionEpoll>(socket_fd));
#endif
}

}
