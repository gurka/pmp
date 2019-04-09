#ifndef TCP_CONNECTION_H_
#define TCP_CONNECTION_H_

class TcpConnection
{
 public:
  virtual ~TcpConnection() = default;

  virtual void close() = 0;
};

#endif  // TCP_CONNECTION_H_
