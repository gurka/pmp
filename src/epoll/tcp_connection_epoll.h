#ifndef TCP_CONNECTION_EPOLL_H_
#define TCP_CONNECTION_EPOLL_H_

#include "tcp_connection.h"

class TcpConnectionEpoll : public TcpConnection
{
 public:
  TcpConnectionEpoll(int socket_fd);

  void close() override;

 private:
  int m_socket_fd;
};

#endif  // TCP_CONNECTION_EPOLL_H_
