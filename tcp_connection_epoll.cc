#include "tcp_connection_epoll.h"

#include <unistd.h>

TcpConnectionEpoll::TcpConnectionEpoll(int socket_fd)
    : m_socket_fd(socket_fd)
{
}

void TcpConnectionEpoll::close()
{
  if (m_socket_fd >= 0)
  {
    ::close(m_socket_fd);
  }
  m_socket_fd = -1;
}
