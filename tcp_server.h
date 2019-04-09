#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <functional>
#include <memory>
#include <cstdint>

class TcpConnection;

class TcpServer
{
 public:
  using OnAccept = std::function<void(std::unique_ptr<TcpConnection>&&)>;

  virtual ~TcpServer() = default;

  virtual void start() = 0;

  static std::unique_ptr<TcpServer> create(std::uint16_t port, const OnAccept& on_accept);
};

#endif  // TCP_SERVER_H_
