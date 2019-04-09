#ifndef TCP_CONNECTION_H_
#define TCP_CONNECTION_H_

#include <functional>
#include <cstdint>

class TcpConnection
{
 public:
  using OnRead   = std::function<bool(const std::uint8_t*, int)>;
  using OnWrite  = std::function<void(void)>;
  using OnError  = std::function<void(const std::string&)>;

  virtual ~TcpConnection() = default;

  virtual void start(const OnRead& on_read,
                     const OnWrite& on_write,
                     const OnError& on_error) = 0;
  virtual void write(const std::uint8_t* buffer, int len) = 0;
  virtual void close() = 0;
};

#endif  // TCP_CONNECTION_H_
