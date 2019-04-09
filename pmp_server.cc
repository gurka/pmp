#include <string>

#include <cstdlib>
#include <cstdio>

#include <unistd.h>

#include "tcp_server.h"
#include "tcp_connection.h"

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

  // Create and start TCP server
  // It will run forever
  // TODO: catch ^C and break
  auto tcp_server = TcpServer::create();
  tcp_server->start(port, [](TcpConnection&& connection)
  {
    printf("New connection!\n");
    connection.close();
  });

  return EXIT_SUCCESS;
}
