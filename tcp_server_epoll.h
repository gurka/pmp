#ifndef TCP_SERVER_EPOLL_H_
#define TCP_SERVER_EPOLL_H_

#include "tcp_server.h"

#include <cstdint>

class TcpServerEpoll : public TcpServer
{
 public:
  void start(std::uint16_t port, const OnAccept& onAccept) override;
};

#endif  // TCP_SERVER_EPOLL_H_
