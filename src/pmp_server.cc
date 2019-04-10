#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>

#include <unistd.h>

#include "tcp_server.h"
#include "tcp_connection.h"
#include "protocol.h"
#include "mandelbrot.h"

static std::unordered_map<int, std::unique_ptr<TcpConnection>> connections;
static std::unordered_map<int, std::vector<std::uint8_t>> responses;
static int next_connection_id = 0;

static void send_response(int connection_id)
{
  if (responses.count(connection_id) == 0)
  {
    fprintf(stderr, "%s: error: no pixels found for connection_id=%d\n", __func__, connection_id);
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

static bool on_read(int connection_id, const std::uint8_t* buffer, int len)
{
  Protocol::Request request;
  if (len == sizeof(request))
  {
    std::copy(buffer, buffer + len, reinterpret_cast<std::uint8_t*>(&request));

    printf("%s: connection_id=%d request: min_c_re=%f min_c_im=%f max_c_re=%f max_c_im=%f x=%d y=%d inf_n=%d\n",
           __func__,
           connection_id,
           request.min_c_re,
           request.min_c_im,
           request.max_c_re,
           request.max_c_im,
           request.x,
           request.y,
           request.inf_n);

    // TODO: Check and print time taken to call compute
    const auto pixels = Mandelbrot::compute(request.min_c_re,
                                            request.min_c_im,
                                            request.max_c_re,
                                            request.max_c_im,
                                            request.x,
                                            request.y,
                                            request.inf_n);

    // Add (move) pixels into the responses map
    responses[connection_id] = std::move(pixels);

    // Start sending response back to client
    send_response(connection_id);
  }
  else
  {
    fprintf(stderr, "Unexpected data length (received=%d expected=%d)\n", len, static_cast<int>(sizeof(request)));
    connections[connection_id]->close();
    connections.erase(connection_id);
  }

  // Return false to tell TcpConnection not continue reading data
  return false;
}

static void on_write(int connection_id)
{
  printf("%s: connection_id=%d\n", __func__, connection_id);

  if (responses.count(connection_id) == 1)
  {
    // There are more pixels to send
    send_response(connection_id);
  }
  else
  {
    // All sent, close and erase the connection
    connections[connection_id]->close();
    connections.erase(connection_id);
  }
}

static void on_error(int connection_id, const std::string& message)
{
  printf("%s: connection_id=%d message=%s\n", __func__, connection_id, message.c_str());
  connections[connection_id]->close();
  connections.erase(connection_id);
}

static void on_accept(std::unique_ptr<TcpConnection>&& connection)
{
  // Save connection
  const auto connection_id = next_connection_id++;
  connections[connection_id] = std::move(connection);
  printf("%s: connection_id=%d\n", __func__, connection_id);

  // and start it with callbacks to on_read, on_write and on_error
  const auto read  = [connection_id](const std::uint8_t* buffer, int len) { return on_read(connection_id, buffer, len); };
  const auto write = [connection_id]()                                    { on_write(connection_id);                    };
  const auto error = [connection_id](const std::string& message)          { on_error(connection_id, message);           };
  connections[connection_id]->start(read, write, error);
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
    fprintf(stderr, "Could not parse given port.\n");
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

  printf("Using port: %d\n", port);

  // Create TCP server
  auto tcp_server = TcpServer::create(port);
  if (!tcp_server)
  {
    // Error message printed by TcpServer::create
    return EXIT_FAILURE;
  }

  // Start server with a callback to on_accept
  // It will run forever
  // TODO: catch ^C and break
  tcp_server->start(on_accept);

  return EXIT_SUCCESS;
}
