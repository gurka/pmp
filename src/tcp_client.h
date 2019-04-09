#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <functional>
#include <memory>
#include <string>

class TcpConnection;

class TcpClient
{
 public:
  using OnConnected = std::function<void(std::unique_ptr<TcpConnection>&&)>;
  using OnError     = std::function<void(const std::string&)>;

  virtual ~TcpClient() = default;

  virtual void start(const OnConnected& on_connected, const OnError& on_error) = 0;

  static std::unique_ptr<TcpClient> create(const std::string& address, const std::string& port);
};

#endif  // TCP_CLIENT_H_
