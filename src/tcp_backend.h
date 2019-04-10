#ifndef TCP_BACKEND_H_
#define TCP_BACKEND_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace TcpBackend
{

class Connection;
class Client;
class Server;

// Callback types
using OnRead      = std::function<bool(const std::uint8_t*, int)>;
using OnWrite     = std::function<void(void)>;
using OnError     = std::function<void(const std::string&)>;
using OnConnected = std::function<void(std::unique_ptr<Connection>&&)>;
using OnAccept    = std::function<void(std::unique_ptr<Connection>&&)>;

/**
 * @brief Creates a TCP client
 *
 * @param[in]  address       Address to connect on
 * @param[in]  port          Port to connect on
 * @param[in]  on_connected  Callback that is called when the client has connected
 * @param[in]  on_error      Callback that is called on error
 *
 * @return Client wrapped in std::unique_ptr, or an empty std::unique_ptr on error
 */
std::unique_ptr<Client> create_client(const std::string& address,
                                      const std::string& port,
                                      const OnConnected& on_connected,
                                      const OnError& on_error);


/**
 * @brief Creates a TCP server
 *
 * @param[in]  port       Port to listen on
 * @param[in]  on_accept  Callback that is called when a client has connected
 *
 * @return Server wrapped in std::unique_ptr, or an empty std::unique_ptr on error
 */
std::unique_ptr<Server> create_server(std::uint16_t port,
                                      const OnAccept& on_accept);

/**
 * @brief Run the TCP backend
 *
 * Async tasks that have been started via calls to e.g. create_client or create_server
 * will be handled in this call. This call will only return when there are no more active
 * async tasks.
 */
void run();

class Connection
{
 public:
  virtual ~Connection() = default;

  virtual void start(const OnRead& on_read,
                     const OnWrite& on_write,
                     const OnError& on_error) = 0;
  virtual void write(const std::uint8_t* buffer, int len) = 0;
  virtual void close() = 0;
};

class Client
{
 public:
  virtual ~Client() = default;
};


class Server
{
 public:
  virtual ~Server() = default;
};

}

#endif  // TCP_BACKEND_H_
