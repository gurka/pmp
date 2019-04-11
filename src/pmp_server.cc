#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <complex>
#include <string>
#include <unordered_map>

#include <unistd.h>

#include "tcp_backend.h"
#include "protocol.h"
#include "mandelbrot.h"
#include "logger.h"

static std::unique_ptr<TcpBackend::Server> server;
static std::unordered_map<int, std::unique_ptr<TcpBackend::Connection>> connections;
static std::unordered_map<int, std::vector<std::uint8_t>> responses;

static void send_response(int connection_id)
{
  if (responses.count(connection_id) == 0)
  {
    LOG_ERROR("%s: error: no pixels found for connection_id=%d", __func__, connection_id);
    return;
  }

  // Get pixels
  auto& pixels = responses[connection_id];

  // Check if there is enough room to send rest of the pixels in one response message
  Protocol::Response response;
  if (pixels.size() <= sizeof(response.pixels))
  {
    // Send rest of pixels
    std::copy(pixels.begin(), pixels.end(), response.pixels);
    response.num_pixels = pixels.size();
    response.last_message = 1;

    // Erase response from map
    responses.erase(connection_id);
  }
  else
  {
    // Send as many pixels as we can
    std::copy(pixels.begin(), pixels.begin() + sizeof(response.pixels), response.pixels);
    response.num_pixels = sizeof(response.pixels);
    response.last_message = 0;

    // Erase the pixels we sent from the vector
    pixels.erase(pixels.begin(), pixels.begin() + sizeof(response.pixels));
  }

  // Send the Response
  connections[connection_id]->write(reinterpret_cast<const std::uint8_t*>(&response), sizeof(response));
}

static void on_disconnected(int connection_id)
{
  LOG_INFO("%s: connection_id=%d", __func__, connection_id);

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

static void on_read(int connection_id, const std::uint8_t* buffer, int len)
{
  Protocol::Request request;
  if (len != sizeof(request))
  {
    LOG_ERROR("%s: unexpected data length (received=%d expected=%d)", __func__, len, static_cast<int>(sizeof(request)));

    // TODO: try to recover instead of just aborting
    exit(EXIT_FAILURE);
  }

  std::copy(buffer, buffer + len, reinterpret_cast<std::uint8_t*>(&request));

  LOG_INFO("%s: connection_id=%d request: min_c_re=%f min_c_im=%f max_c_re=%f max_c_im=%f image_width=%d image_height=%d max_iter=%d",
           __func__,
           connection_id,
           request.min_c_re,
           request.min_c_im,
           request.max_c_re,
           request.max_c_im,
           request.image_width,
           request.image_height,
           request.max_iter);

  // TODO: Check and print time taken to call compute
  const auto pixels = Mandelbrot::compute(std::complex<double>(request.min_c_re, request.min_c_im),
                                          std::complex<double>(request.max_c_re, request.max_c_im),
                                          request.image_width,
                                          request.image_height,
                                          request.max_iter);

  // Add (move) pixels into the responses map
  responses[connection_id] = std::move(pixels);

  // Start sending response back to client
  send_response(connection_id);
}

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
    // Restart read procedure to see if the client has more requests
    connections[connection_id]->read();
  }
}

static void on_error(int connection_id, const std::string& message)
{
  LOG_ERROR("%s: connection_id=%d message=%s", __func__, connection_id, message.c_str());

  // TODO: try to recover instead of just aborting
  exit(EXIT_FAILURE);
}

static void on_accept(std::unique_ptr<TcpBackend::Connection>&& connection)
{
  // Unique id per connection
  static int next_connection_id = 0;
  const auto connection_id = next_connection_id++;

  LOG_INFO("%s: connection_id=%d", __func__, connection_id);

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

  // Verify port number here
  // We could skip checking the port number here and let
  // it fail at bind() or listen() instead, but by checking
  // here we can control and print similar error messages

  // Valid TCP ports are 1..65535
  if (port <= 0 || port > 65535)
  {
    fprintf(stderr, "Given port (%d) is out of range.\n", port);
    return EXIT_FAILURE;
  }

  // On linux, port 1..1023 requires root (effective uid 0)
  if (port < 1024 && geteuid() != 0)
  {
    fprintf(stderr, "Given port (%d) requires root.\n", port);
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

  // Start network backend
  // It will run forever
  // TODO: catch ^C and break
  TcpBackend::run();

  return EXIT_SUCCESS;
}
