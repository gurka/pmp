#ifndef TCP_CONNECTION_EPOLL_H_
#define TCP_CONNECTION_EPOLL_H_

#include "tcp_backend.h"

namespace TcpBackend
{

class ConnectionEpoll : public Connection
{
 public:
  ConnectionEpoll(int socket_fd);

  void set_callbacks(const OnDisconnected& on_disconnected,
                     const OnRead& on_read,
                     const OnWrite& on_write,
                     const OnError& on_error) override;
  void read() override;
  void write(const std::uint8_t* buffer, int len) override;
  void close() override;

 private:
  int m_socket_fd;

  OnDisconnected m_on_disconnected;
  OnRead         m_on_read;
  OnWrite        m_on_write;
  OnError        m_on_error;
};

}

#endif  // TCP_CONNECTION_EPOLL_H_
