#ifndef TCP_SERVER_EPOLL_H_
#define TCP_SERVER_EPOLL_H_

#include "tcp_backend.h"

#include <cstdint>

namespace TcpBackend
{

class ServerEpoll : public Server
{
 public:
  ServerEpoll(int socket_fd, const OnAccept& on_accept);

  void accept() override;

 private:
  int m_socket_fd;
  OnAccept m_on_accept;
};

}

#endif  // TCP_SERVER_EPOLL_H_
