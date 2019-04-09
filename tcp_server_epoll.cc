#include "tcp_server_epoll.h"

// static creator
std::unique_ptr<TcpServer> TcpServer::create()
{
  return std::make_unique<TcpServerEpoll>();
}

void TcpServerEpoll::start(std::uint16_t port, const OnAccept& onAccept)
{
  (void)port;
  (void)onAccept;
}
