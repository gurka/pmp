#include <cstdio>
#include <string>
#include <memory>

#include "tcp_client.h"
#include "tcp_connection.h"
#include "protocol.h"
#include "pgm.h"

static std::unique_ptr<TcpConnection> connection;

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

    // TODO: handle case where this isn't the last message

    PGM::write_pgm("test.pgm", 100, 100, response.pixels);
  }
  else
  {
    fprintf(stderr, "Unexpected data length (received=%d expected=%d)\n", len, static_cast<int>(sizeof(response)));
  }

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
  request.min_c_re = -1.0f;
  request.min_c_im = -1.5f;
  request.max_c_re =  2.0f;
  request.max_c_im =  1.5f;
  request.x = 100;
  request.y = 100;
  request.inf_n = 1024;
  connection->write(reinterpret_cast<std::uint8_t*>(&request), sizeof(request));
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s ADDRESS:PORT\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Split argument into address and port
  const auto arg = std::string(argv[1]);
  const auto sep = arg.find(":");
  if (sep == std::string::npos || sep == arg.size() - 1)
  {
    fprintf(stderr, "usage: %s ADDRESS:PORT\n", argv[0]);
    return EXIT_FAILURE;
  }

  const auto address = arg.substr(0, sep);
  const auto port = arg.substr(sep + 1);
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
