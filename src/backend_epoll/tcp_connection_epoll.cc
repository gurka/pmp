#include "tcp_connection_epoll.h"

#include <unistd.h>

#include "logger.h"

namespace TcpBackend
{

ConnectionEpoll::ConnectionEpoll(int socket_fd)
    : m_socket_fd(socket_fd)
{
}

void ConnectionEpoll::set_callbacks(const OnDisconnected& on_disconnected,
                                    const OnRead& on_read,
                                    const OnWrite& on_write,
                                    const OnError& on_error)
{
  m_on_disconnected = on_disconnected;
  m_on_read         = on_read;
  m_on_write        = on_write;
  m_on_error        = on_error;
}

void ConnectionEpoll::read()
{
  LOG_ERROR("%s: not yet implemented", __func__);
}

void ConnectionEpoll::write(const std::uint8_t* buffer, int len)
{
  LOG_ERROR("%s: not yet implemented", __func__);
  (void)buffer;
  (void)len;
}

void ConnectionEpoll::close()
{
  if (m_socket_fd >= 0)
  {
    ::close(m_socket_fd);
  }
  m_socket_fd = -1;
}

}
