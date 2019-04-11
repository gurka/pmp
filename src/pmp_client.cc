#include <cstdio>
#include <cmath>
#include <complex>
#include <deque>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "tcp_backend.h"
#include "protocol.h"
#include "pgm.h"

static struct Arguments
{
  std::complex<double> min_c;
  std::complex<double> max_c;
  int image_width;
  int image_height;
  int max_iter;
  int divisions;
} arguments;

static std::deque<Protocol::Request> request_queue;
static std::vector<std::uint8_t> image_pixels;

struct Session
{
  std::unique_ptr<TcpBackend::Connection> connection;
  Protocol::Request current_request;
  std::vector<std::uint8_t> current_pixels;
};
static std::unordered_map<int, Session> sessions;

static void send_request(int session_id)
{
  auto& session = sessions[session_id];

  // Fetch and remove next request from queue
  session.current_request = request_queue.front();
  request_queue.pop_front();

  // Make sure to clear ongoing_pixels
  session.current_pixels.clear();

  // And send it
  session.connection->write(reinterpret_cast<const std::uint8_t*>(&session.current_request),
                            sizeof(session.current_request));
}

static bool on_read(int session_id, const std::uint8_t* buffer, int len)
{
  auto& session = sessions[session_id];

  printf("%s: session_id=%d len=%d\n", __func__, session_id, len);

  Protocol::Response response;
  if (len != sizeof(response))
  {
    fprintf(stderr, "Unexpected data length (received=%d expected=%d)\n", len, static_cast<int>(sizeof(response)));

    // Unrecoverable... for now
    exit(EXIT_FAILURE);
  }

  std::copy(buffer, buffer + len, reinterpret_cast<std::uint8_t*>(&response));

  printf("%s: response: num_pixels=%d last_message=%d\n",
         __func__,
         response.num_pixels,
         response.last_message);

  // Add the pixels we received
  session.current_pixels.insert(session.current_pixels.end(), response.pixels, response.pixels + response.num_pixels);

  if (response.last_message == 0)
  {
    // Read more messages
    return true;
  }

  // Add current_pixels to image_pixels, at the correct position
  // TODO: Simplify this, if possible
  const auto sub_c = (arguments.max_c - arguments.min_c) / static_cast<double>(arguments.divisions);
  const auto dx = (session.current_request.min_c_re - arguments.min_c.real()) / sub_c.real();
  const auto dy = (session.current_request.min_c_im - arguments.min_c.imag()) / sub_c.imag();
  auto to = image_pixels.begin() +
            (session.current_request.image_height * dy * arguments.image_width) +
            (session.current_request.image_width * dx);
  auto from = session.current_pixels.begin();
  for (auto y = 0; y < session.current_request.image_height; y++)
  {
    std::copy(from, from + session.current_request.image_width, to);
    from += session.current_request.image_width;
    to += arguments.image_width;
  }

  // Check if there are more requests to handle
  if (!request_queue.empty())
  {
    // Handle next request and continue to read messages
    send_request(session_id);
    return true;
  }

  // Close connection
  sessions[session_id].connection->close();
  sessions.erase(session_id);

  // If this was the last session to be closed then we can write the image and we're done!
  if (sessions.empty())
  {
    printf("%s: no more requests and no more sessions, writing image\n", __func__);
    PGM::write_pgm("test.pgm", arguments.image_width, arguments.image_height, image_pixels.data());
  }

  return false;
}

static void on_write(int session_id)
{
  printf("%s: session_id=%d\n", __func__, session_id);
}

static void on_error_connection(int session_id, const std::string& message)
{
  fprintf(stderr, "%s: session_id=%d message=%s\n", __func__, session_id, message.c_str());
  sessions[session_id].connection->close();
  sessions.erase(session_id);
}

static void on_error_client(const std::string& message)
{
  fprintf(stderr, "%s: message=%s\n", __func__, message.c_str());
}

static void on_connected(std::unique_ptr<TcpBackend::Connection>&& connection)
{
  static int next_session_id = 0;
  const auto session_id = next_session_id;
  next_session_id += 1;

  printf("%s: session_id=%d\n", __func__, session_id);

  // Save new session
  Session session;
  session.connection = std::move(connection);
  sessions.emplace(session_id, std::move(session));

  // Create callbacks and start connection
  auto read = [session_id](const std::uint8_t* buffer, int len)
  {
    return on_read(session_id, buffer, len);
  };
  auto write = [session_id]()
  {
    on_write(session_id);
  };
  auto error = [session_id](const std::string& message)
  {
    on_error_connection(session_id, message);
  };
  sessions[session_id].connection->start(read, write, error);

  // Send next request in queue to this server
  send_request(session_id);
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
    const auto min_c_re = std::stod(argv[1]);
    const auto min_c_im = std::stod(argv[2]);
    const auto max_c_re = std::stod(argv[3]);
    const auto max_c_im = std::stod(argv[4]);
    arguments.min_c        = std::complex<double>(min_c_re, min_c_im);
    arguments.max_c        = std::complex<double>(max_c_re, max_c_im);
    arguments.max_iter     = std::stoi(argv[5]);
    arguments.image_width  = std::stoi(argv[6]);
    arguments.image_height = std::stoi(argv[7]);
    arguments.divisions    = std::stoi(argv[8]);
  }
  catch (const std::exception& e)
  {
    fprintf(stderr, "%s\n", e.what());
    fprintf(stderr, "usage: %s min_c_re min_c_im max_c_re max_c_im max_n x y divisions list-of-servers\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Create requests based on options
  const auto sub_x = arguments.image_width / arguments.divisions;
  const auto sub_y = arguments.image_height / arguments.divisions;
  const auto sub_c = (arguments.max_c - arguments.min_c) / static_cast<double>(arguments.divisions);

  // Add first request
  Protocol::Request request;
  request.min_c_re     = arguments.min_c.real();
  request.min_c_im     = arguments.min_c.imag();
  request.max_c_re     = arguments.min_c.real() + sub_c.real();
  request.max_c_im     = arguments.min_c.imag() + sub_c.imag();
  request.image_width  = sub_x;
  request.image_height = sub_y;
  request.max_iter     = arguments.max_iter;
  request_queue.push_back(request);

  // Add rest of the requests
  for (auto i = 1; i < std::pow(arguments.divisions, 2); i++)
  {
    request.min_c_re += sub_c.real();
    request.max_c_re += sub_c.real();
    if (request.max_c_re > arguments.max_c.real())
    {
      // Reset re and increase im
      request.min_c_re = arguments.min_c.real();
      request.max_c_re = arguments.min_c.real() + sub_c.real();
      request.min_c_im += sub_c.imag();
      request.max_c_im += sub_c.imag();
    }
    request_queue.push_back(request);
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

  // Pre-allocate total_pixels vector
  image_pixels.insert(image_pixels.begin(), arguments.image_width * arguments.image_height, 0u);

  // Create one TCP client per server
  std::vector<std::unique_ptr<TcpBackend::Client>> clients;
  for (const auto& server : servers)
  {
    const auto address = std::get<0>(server);
    const auto port = std::get<1>(server);

    printf("Creating TCP client to address %s port %s\n", address.c_str(), port.c_str());

    // Create TCP client
    auto tcp_client = TcpBackend::create_client(address, port, on_connected, on_error_client);
    if (tcp_client)
    {
      clients.push_back(std::move(tcp_client));
    }
  }

  if (clients.empty())
  {
    fprintf(stderr, "No TCP client could be created successfully\n");
    return EXIT_FAILURE;
  }

  // Start network backend
  // It will run forever
  // TODO: catch ^C and break
  TcpBackend::run();

  return EXIT_SUCCESS;
}
