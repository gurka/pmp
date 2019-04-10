#include <cstdio>
#include <string>
#include <memory>
#include <vector>

#include "tcp_client.h"
#include "tcp_connection.h"
#include "protocol.h"
#include "pgm.h"

static struct Options
{
  double min_c_re;
  double min_c_im;
  double max_c_re;
  double max_c_im;
  int max_n;
  int x;
  int y;
} options;
static std::unique_ptr<TcpConnection> connection;
static std::vector<std::uint8_t> pixels;

static bool on_read(const std::uint8_t* buffer, int len)
{
  Protocol::Response response;
  if (len == sizeof(response))
  {
    std::copy(buffer, buffer + len, reinterpret_cast<std::uint8_t*>(&response));

    printf("%s: response: num_pixels=%d last_message=%d\n",
           __func__,
           response.num_pixels,
           response.last_message);

    pixels.insert(pixels.end(), response.pixels, response.pixels + response.num_pixels);

    if (response.last_message == 0)
    {
      // Read more messages
      return true;
    }

    // This was the last message, write the pgm
    PGM::write_pgm("test.pgm", options.x, options.y, pixels.data());

    // Close connection
    connection->close();
    connection.reset();
    return false;
  }

  fprintf(stderr, "Unexpected data length (received=%d expected=%d)\n", len, static_cast<int>(sizeof(response)));
  connection->close();
  connection.reset();
  return false;
}

static void on_write()
{
  puts(__func__);
}

static void on_error(const std::string& message)
{
  puts(message.c_str());

  if (connection)
  {
    connection->close();
  }
}

static void on_connected(std::unique_ptr<TcpConnection>&& connection_)
{
  puts(__func__);

  // Start TcpConnection
  connection = std::move(connection_);
  connection->start(on_read, on_write, on_error);

  // Send Request message
  Protocol::Request request;
  request.min_c_re = options.min_c_re;
  request.min_c_im = options.min_c_im;
  request.max_c_re = options.max_c_re;
  request.max_c_im = options.max_c_im;
  request.x        = options.x;
  request.y        = options.y;
  request.inf_n    = options.max_n;
  connection->write(reinterpret_cast<std::uint8_t*>(&request), sizeof(request));
}

int main(int argc, char* argv[])
{
  if (argc < 10)
  {
    fprintf(stderr, "usage: %s min_c_re min_c_im max_c_re max_c_im max_n x y divisions list-of-servers\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Parse numbers
  int divisions;
  try
  {
    options.min_c_re = std::stod(argv[1]);
    options.min_c_im = std::stod(argv[2]);
    options.max_c_re = std::stod(argv[3]);
    options.max_c_im = std::stod(argv[4]);
    options.max_n    = std::stoi(argv[5]);
    options.x        = std::stoi(argv[6]);
    options.y        = std::stoi(argv[7]);
    divisions        = std::stoi(argv[8]);
  }
  catch (const std::exception& e)
  {
    fprintf(stderr, "%s\n", e.what());
    fprintf(stderr, "usage: %s min_c_re min_c_im max_c_re max_c_im max_n x y divisions list-of-servers\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Parse list of servers
  std::vector<std::tuple<std::string, std::string>> servers;
  for (auto i = 9; i < argc; i++)
  {
    const auto arg = std::string(argv[i]);
    const auto sep = arg.find(":");
    if (sep == std::string::npos || sep == arg.size() - 1)
    {
      fprintf(stderr, "usage: %s min_c_re min_c_im max_c_re max_c_im max_n x y divisions list-of-servers\n", argv[0]);
      return EXIT_FAILURE;
    }

    const auto address = arg.substr(0, sep);
    const auto port = arg.substr(sep + 1);
    servers.emplace_back(address, port);
  }

  // TODO: Use divisions and more than one server

  const auto address = std::get<0>(servers[0]);
  const auto port = std::get<1>(servers[0]);
  printf("Connecting to address %s port %s\n", address.c_str(), port.c_str());

  // Create TCP client
  auto tcp_client = TcpClient::create(address, port);
  if (!tcp_client)
  {
    // Error message printed by TcpServer::create
    return EXIT_FAILURE;
  }

  // Start client with a callback to on_connected
  // It will run forever
  // TODO: catch ^C and break
  tcp_client->start(on_connected, on_error);

  return EXIT_SUCCESS;
}
