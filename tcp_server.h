#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <functional>
#include <memory>
#include <cstdint>

class TcpConnection;

class TcpServer
{
 public:
  using OnAccept = std::function<void(TcpConnection&&)>;

  virtual ~TcpServer() = default;

  virtual void start(std::uint16_t port, const OnAccept& onAccept) = 0;

  static std::unique_ptr<TcpServer> create();
};

#endif  // TCP_SERVER_H_
