#ifndef TCP_BACKEND_H_
#define TCP_BACKEND_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace TcpBackend
{

// Forward declarations

class Connection;
class Server;

// Callback types

/**
 * @brief OnRead callback
 *
 * Called when the Connection has read data
 *
 * @param[in]  data  Pointer to data
 * @param[in]  len   Length of data
 */
using OnRead = std::function<void(const std::uint8_t* data, int len)>;

/**
 * @brief OnWrite callback
 *
 * Called when the Connection has written queued data
 */
using OnWrite = std::function<void(void)>;

/**
 * @brief OnError callback
 *
 * Called by Connection, Server or connect() when an error occurs
 *
 * @param[in]  message  The error message
 */
using OnError = std::function<void(const std::string& message)>;

/**
 * @brief OnConnected callback
 *
 * Called by connect() when it has connected
 *
 * @param[in]  connection  The Connection that represents the connection
 * @param[in]  address     Address that the Connection is connected to
 * @param[in]  port        Port that the Connection is connected to
 */
using OnConnected = std::function<void(std::unique_ptr<Connection>&& connection,
                                       const std::string& address,
                                       const std::string& port)>;

/**
 * @brief OnDisconnected callback
 *
 * Called by Connection when the connection has closed and no more
 * async tasks are ongoing.
 *
 * The Connection instance may only be deleted when this callback
 * has been called.
 *
 * @see Connection for more information.
 */
using OnDisconnected = std::function<void(void)>;

/**
 * @brief OnAccept callback
 *
 * Called by Server when a new connection has been accepted
 *
 * @param[in]  connection  The new Connection
 */
using OnAccept    = std::function<void(std::unique_ptr<Connection>&& connection)>;

// Functions

/**
 * @brief Creates a TCP connection towards the given address and port
 *
 * @param[in]  address       Address to connect on
 * @param[in]  port          Port to connect on
 * @param[in]  on_connected  Callback that is called when the client has connected
 * @param[in]  on_error      Callback that is called on error
 */
void connect(const std::string& address,
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

/**
 * @brief Stop the TCP backend
 *
 * If the TCP backend is running it will be stopped and all ongoing async tasks aborted,
 * finally the call to run() will return.
 */
void stop();

/**
 * @brief Represents a connection
 *
 * Created by connect() or Server when a connection has been established
 */
class Connection
{
 public:
  virtual ~Connection() = default;

  /**
   * @brief Set callbacks
   *
   * These callbacks must be set before any read or write
   * procedures are started.
   *
   * @param[in]  on_disconnected  OnDisconnected callback, @see OnDisconnected
   * @param[in]  on_read          OnRead callback, @see OnRead
   * @param[in]  on_write         OnWrite callback, @see OnWrite
   * @param[in]  on_error         OnError callback, @see OnError
   */
  virtual void set_callbacks(const OnDisconnected& on_disconnected,
                             const OnRead& on_read,
                             const OnWrite& on_write,
                             const OnError& on_error) = 0;

  /**
   * @brief Starts read procedure (async)
   *
   * The OnRead callback will be called when data has
   * been read.
   */
  virtual void read() = 0;

  /**
   * @brief Starts write procedure (async)
   *
   * @param[in]  buffer  The data to send
   * @param[in]  len     Length of data
   */
  virtual void write(const std::uint8_t* buffer, int len) = 0;

  /**
   * @brief Closes the connection
   *
   * Any ongoing async tasks will be aborted and the OnDisconnected
   * callback will be called either in this context or in a later context.
   *
   * The Connection instance may only be deleted when the OnDisconnected
   * callback has been called.
   */
  virtual void close() = 0;
};

/**
 * @brief Used to accept new connections
 *
 * @see create_server
 */
class Server
{
 public:
  virtual ~Server() = default;

  /**
   * @brief Start accept procedure (async)
   */
  virtual void accept() = 0;
};

}

#endif  // TCP_BACKEND_H_
