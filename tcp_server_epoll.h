#ifndef TCP_SERVER_EPOLL_H_
#define TCP_SERVER_EPOLL_H_

#include "tcp_server.h"

#include <cstdint>

class TcpServerEpoll : public TcpServer
{
 public:
  TcpServerEpoll(int socket_fd, const OnAccept& on_accept);

  void start() override;

 private:
  int m_socket_fd;
  OnAccept m_on_accept;
};

#endif  // TCP_SERVER_EPOLL_H_
