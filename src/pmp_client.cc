#include <cstdio>
#include <cmath>
#include <string>
#include <memory>
#include <vector>

#include "tcp_client.h"
#include "tcp_connection.h"
#include "protocol.h"
#include "pgm.h"

static struct Arguments
{
  double min_c_re;
  double min_c_im;
  double max_c_re;
  double max_c_im;
  int max_n;
  int x;
  int y;
  int divisions;
} arguments;

static std::vector<Protocol::Request> requests;
static Protocol::Request ongoing_request;
static std::vector<std::uint8_t> ongoing_pixels;
static std::vector<std::uint8_t> total_pixels;

static std::vector<std::tuple<std::string, std::string>> servers;
static std::unique_ptr<TcpConnection> connection;

static void send_request()
{
  // Fetch and remove next request from queue
  ongoing_request = requests.front();
  requests.erase(requests.begin());

  // Make sure to clear ongoing_pixels
  ongoing_pixels.clear();

  // And send it
  connection->write(reinterpret_cast<const std::uint8_t*>(&ongoing_request), sizeof(ongoing_request));
}

static bool on_read(const std::uint8_t* buffer, int len)
{
  Protocol::Response response;
  if (len != sizeof(response))
  {
    fprintf(stderr, "Unexpected data length (received=%d expected=%d)\n", len, static_cast<int>(sizeof(response)));
    exit(EXIT_FAILURE);
  }
  else
  {
    std::copy(buffer, buffer + len, reinterpret_cast<std::uint8_t*>(&response));

    printf("%s: response: num_pixels=%d last_message=%d\n",
           __func__,
           response.num_pixels,
           response.last_message);

    // Add the pixels we received
    ongoing_pixels.insert(ongoing_pixels.end(), response.pixels, response.pixels + response.num_pixels);

    if (response.last_message == 0)
    {
      // Read more messages
      return true;
    }

    // Add ongoing_pixels to total_pixels, at the correct position
    const auto sub_re = (arguments.max_c_re - arguments.min_c_re) / arguments.divisions;
    const auto sub_im = (arguments.max_c_im - arguments.min_c_im) / arguments.divisions;
    const auto dx = (ongoing_request.min_c_re - arguments.min_c_re) / sub_re;
    const auto dy = (ongoing_request.min_c_im - arguments.min_c_im) / sub_im;
    auto to = total_pixels.begin() + (ongoing_request.y * dy * arguments.x) + (ongoing_request.x * dx);
    auto from = ongoing_pixels.begin();
    for (auto y = 0; y < ongoing_request.y; y++)
    {
      std::copy(from, from + ongoing_request.x, to);
      from += ongoing_request.x;
      to += arguments.x;
    }
  }

  // Check if there are more requests to handle
  if (requests.empty())
  {
    // TODO: this wont work when there are multiple connections/servers
    printf("%s: no more requests, writing image\n", __func__);
    PGM::write_pgm("test.pgm", arguments.x, arguments.y, total_pixels.data());

    // Close connection
    connection->close();
    connection.reset();
    return false;
  }

  // Handle next request and continue to read messages
  send_request();
  return true;
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

  send_request();
}

int main(int argc, char* argv[])
{
  if (argc < 10)
  {
    fprintf(stderr, "usage: %s min_c_re min_c_im max_c_re max_c_im max_n x y divisions list-of-servers\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Parse options
  try
  {
    arguments.min_c_re  = std::stod(argv[1]);
    arguments.min_c_im  = std::stod(argv[2]);
    arguments.max_c_re  = std::stod(argv[3]);
    arguments.max_c_im  = std::stod(argv[4]);
    arguments.max_n     = std::stoi(argv[5]);
    arguments.x         = std::stoi(argv[6]);
    arguments.y         = std::stoi(argv[7]);
    arguments.divisions = std::stoi(argv[8]);
  }
  catch (const std::exception& e)
  {
    fprintf(stderr, "%s\n", e.what());
    fprintf(stderr, "usage: %s min_c_re min_c_im max_c_re max_c_im max_n x y divisions list-of-servers\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Create requests based on options
  const auto sub_x = arguments.x / arguments.divisions;
  const auto sub_y = arguments.y / arguments.divisions;
  const auto sub_re = (arguments.max_c_re - arguments.min_c_re) / arguments.divisions;
  const auto sub_im = (arguments.max_c_im - arguments.min_c_im) / arguments.divisions;

  // Add first request
  Protocol::Request request;
  request.min_c_re = arguments.min_c_re;
  request.min_c_im = arguments.min_c_im;
  request.max_c_re = arguments.min_c_re + sub_re;
  request.max_c_im = arguments.min_c_im + sub_im;
  request.inf_n = arguments.max_n;
  request.x = sub_x;
  request.y = sub_y;
  requests.push_back(request);

  // Add rest of the requests
  for (auto i = 1; i < std::pow(arguments.divisions, 2); i++)
  {
    request.min_c_re += sub_re;
    request.max_c_re += sub_re;
    if (request.max_c_re > arguments.max_c_re)
    {
      // Reset re and increase im
      request.min_c_re = arguments.min_c_re;
      request.max_c_re = arguments.min_c_re + sub_re;
      request.min_c_im += sub_im;
      request.max_c_im += sub_im;
    }
    requests.push_back(request);
  }

  // Parse list of servers
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

  // Pre-allocate total_pixels vector
  total_pixels.insert(total_pixels.begin(), arguments.x * arguments.y, 0u);

  // TODO: Use more than one server
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
