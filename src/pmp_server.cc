#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <complex>
#include <string>
#include <unordered_map>

#include "tcp_backend.h"
#include "protocol.h"
#include "mandelbrot.h"
#include "logger.h"

// The server
static std::unique_ptr<TcpBackend::Server> server;

// Map connection_id -> Connection
static std::unordered_map<int, std::unique_ptr<TcpBackend::Connection>> connections;

// Map connection_id -> Pixels
static std::unordered_map<int, std::vector<std::uint8_t>> responses;

/**
 * @brief Send (or continue to send) Response to given Connection
 *
 * Takes as many pixels as possible (Response message has a max size)
 * from responses and sends it to the Connection in a Response message.
 *
 * @param[in]  connection_id  Id of the connection to which send Response
 */
static void send_response(int connection_id)
{
  if (responses.count(connection_id) == 0)
  {
    LOG_ERROR("%s: error: no pixels found for connection_id=%d", __func__, connection_id);
    return;
  }

  // Get pixels
  auto& pixels = responses[connection_id];

  // Maximum size of byte array (pixels) in messages is 2^15 = 32768
  // so we need to split the response into multiple messages if we have
  // more pixels than that
  Protocol::Response response;
  if (pixels.size() <= (1u << 15))
  {
    // We can send all pixels in this message
    // Serialize and send the message
    response.pixels = responses[connection_id];
    response.last_message = true;
    const auto buffer = Protocol::serialize(response);
    connections[connection_id]->write(buffer.data(), buffer.size());

    // Erase the response/pixels
    responses.erase(connection_id);
  }
  else
  {
    // Send as many pixels as possible (2^15)
    response.pixels.insert(response.pixels.end(),
                           pixels.begin(),
                           pixels.begin() + (1u << 15));
    response.last_message = false;
    const auto buffer = Protocol::serialize(response);
    connections[connection_id]->write(buffer.data(), buffer.size());

    // Erase the pixels we sent from the vector
    pixels.erase(pixels.begin(), pixels.begin() + (1u << 15));
  }
}

/**
 * @brief Callback called when a connection disconnects
 *
 * Disconnect is expected only when the response has been sent,
 * if this is not the case we abort the program.
 *
 * @param[in]  connection_id  Id of the connection that disconnected
 */
static void on_disconnected(int connection_id)
{
  LOG_INFO("Connection %d disconnected", connection_id);

  // Delete the connection
  connections.erase(connection_id);

  // If the client disconnected unexpectedly (response still in queue) then
  // we have to abort
  if (responses.count(connection_id) == 1)
  {
    LOG_ERROR("%s: unexpected disconnect, response not sent", __func__);

    // TODO: try to recover instead of just aborting
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief Callback called when a connection has read a message
 *
 * The only message that is read is Request, so if we cannot deserialize
 * as a Request then abort.
 *
 * The Mandelbrot pixels are computated and added to the responses variable
 * and we then start sending a response to the connection.
 *
 * @param[in]  connection_id  Id of the connection that has read a message
 * @param[in]  buffer         Message data
 * @param[in]  len            Length of message data
 */
static void on_read(int connection_id, const std::uint8_t* buffer, int len)
{
  Protocol::Request request;
  if (!Protocol::deserialize(std::vector<std::uint8_t>(buffer, buffer + len), &request))
  {
    LOG_ERROR("%s: could not deseralize Request message", __func__);

    // TODO: try to recover instead of just aborting
    exit(EXIT_FAILURE);
  }

  LOG_INFO("Received request from connection %d: (%.2lf, %.2lf)..(%.2lf, %.2lf) (%d, %d) %d",
           connection_id,
           request.min_c.real(),
           request.min_c.imag(),
           request.max_c.real(),
           request.max_c.imag(),
           request.image_width,
           request.image_height,
           request.max_iter);

  const auto time_begin = std::chrono::steady_clock::now();
  const auto pixels = Mandelbrot::compute(request.min_c,
                                          request.max_c,
                                          request.image_width,
                                          request.image_height,
                                          request.max_iter);
  const auto time_end = std::chrono::steady_clock::now();

  LOG_INFO("The request from connection %d took %dms (%ds) to compute",
           connection_id,
           std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_begin).count(),
           std::chrono::duration_cast<std::chrono::seconds>(time_end - time_begin).count());

  // Add (move) pixels into the responses map
  responses[connection_id] = std::move(pixels);

  // Start sending response back to client
  send_response(connection_id);
}

/**
 * @brief Callback called when a session has written a message
 *
 * If the connection has more pixels to send we do that, otherwise
 * we re-start the read procedure to see if the client has more
 * requests.
 *
 * @param[in]  connection_id  Id of the connection that has written a message
 */
static void on_write(int connection_id)
{
  LOG_DEBUG("%s: connection_id=%d", __func__, connection_id);

  // Check if there are more pixels to send
  if (responses.count(connection_id) == 1)
  {
    send_response(connection_id);
  }
  else
  {
    LOG_INFO("Response successfully sent to connection %d", connection_id);

    // Restart read procedure to see if the client has more requests
    connections[connection_id]->read();
  }
}

/**
 * @brief Callback called when an error occurs in a connection
 *
 * Errors are currently not handled and the program is aborted on any
 * kind of error.
 *
 * @param[in]  connection_id  Id of the connection for which an error occurred
 * @param[in]  message        Error message
 */
static void on_error(int connection_id, const std::string& message)
{
  LOG_ERROR("%s: connection_id=%d message=%s", __func__, connection_id, message.c_str());

  // TODO: try to recover instead of just aborting
  exit(EXIT_FAILURE);
}

/**
 * @brief Callback called when the Server has accepted a connection
 *
 * The connection is initialized and the read procedure is started.
 * We also tell the server to continue accepting new connections.
 *
 * @param[in]  connection  The connection, wrapped in std::unique_ptr
 */
static void on_accept(std::unique_ptr<TcpBackend::Connection>&& connection)
{
  // Unique id per connection
  static int next_connection_id = 0;
  const auto connection_id = next_connection_id++;

  LOG_INFO("Connection %d connected", connection_id);

  // Save connection
  connections[connection_id] = std::move(connection);

  // Set callbacks
  const auto disconnected  = [connection_id]()                                    { on_disconnected(connection_id);      };
  const auto read          = [connection_id](const std::uint8_t* buffer, int len) { on_read(connection_id, buffer, len); };
  const auto write         = [connection_id]()                                    { on_write(connection_id);             };
  const auto error         = [connection_id](const std::string& message)          { on_error(connection_id, message);    };
  connections[connection_id]->set_callbacks(disconnected, read, write, error);

  // Start read procedure
  connections[connection_id]->read();

  // Continue to accept new connections
  server->accept();
}

/**
 * @brief Main
 *
 * Parses arguments.
 * Creates and starts Server.
 * Runs forever.
 *
 * @param[in]  argc  argc
 * @param[in]  argv  argv
 *
 * @return exit code
 */
int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s PORT\n", argv[0]);
    return EXIT_FAILURE;
  }

  int port = 0;
  try
  {
    port = std::stoi(argv[1]);
  }
  catch (const std::exception& e)
  {
    fprintf(stderr, "exception: %s\n", e.what());
    fprintf(stderr, "usage: %s PORT\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Valid TCP ports are 1..65535
  if (port <= 0 || port > 65535)
  {
    fprintf(stderr, "Given port (%d) is out of range.\n", port);
    return EXIT_FAILURE;
  }

  LOG_INFO("Listening on port: %d", port);

  // Create and start TCP server
  server = TcpBackend::create_server(port, on_accept);
  if (!server)
  {
    // Error message printed by TcpBackend::create_server
    return EXIT_FAILURE;
  }
  server->accept();

  // Start network backend, it will run until there are no more
  // active async tasks
  // TODO: catch ^C and quit gracefully
  TcpBackend::run();

  return EXIT_SUCCESS;
}
