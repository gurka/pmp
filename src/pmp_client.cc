#include <cstdio>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <complex>
#include <deque>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "tcp_backend.h"
#include "protocol.h"
#include "pgm.h"
#include "logger.h"

// Program arguments, parsed and set in main()
static struct Arguments
{
  std::complex<double> min_c;
  std::complex<double> max_c;
  int image_width;
  int image_height;
  int max_iter;
  int divisions;
} arguments;

// Queue of Requests, based on arguments and created in main()
static std::deque<Protocol::Request> request_queue;

// The final image's pixels (8bpp)
static std::vector<std::uint8_t> image_pixels;

// Represents a session/connection to a server
struct Session
{
  std::unique_ptr<TcpBackend::Connection> connection;
  bool request_ongoing;
  Protocol::Request current_request;
  std::vector<std::uint8_t> current_pixels;
};

// All Sessions, mapped with an unique id
static std::unordered_map<int, Session> sessions;

/**
 * @brief Send the next request in the queue
 *
 * Take and remove the next Request in the queue and
 * send it using the given session_id.
 *
 * Assumes that the queue is not empty and that a Session
 * with the given session_id exist.
 *
 * @param[in]  session_id  The session to use for next request
 */
static void send_request(int session_id)
{
  auto& session = sessions[session_id];

  // Make sure to clear ongoing_pixels
  session.request_ongoing = true;
  session.current_pixels.clear();

  // Fetch and remove next request from queue
  session.current_request = request_queue.front();
  request_queue.pop_front();

  LOG_INFO("Session %d sends request (%.2lf, %.2lf)..(%.2lf, %.2lf) (%d, %d) %d",
           session_id,
           session.current_request.min_c.real(),
           session.current_request.min_c.imag(),
           session.current_request.max_c.real(),
           session.current_request.max_c.imag(),
           session.current_request.image_width,
           session.current_request.image_height,
           session.current_request.max_iter);

  // Send the request
  const auto buffer = Protocol::serialize(session.current_request);
  session.connection->write(buffer.data(), buffer.size());

  // And start reading response messages
  session.connection->read();
}

/**
 * @brief Callback called when a session disconnects
 *
 * Disconnect is expected only when the Request queue is empty
 * and there is no ongoing request for the given Session.
 *
 * If any of the above is not true the program is aborted,
 * Otherwise the Session is deleted. If this was the last Session
 * to disconnect then we'll write the final image.
 *
 * @param[in]  session_id  Id of the session that disconnected
 */
static void on_disconnected(int session_id)
{
  LOG_INFO("Session %d disconnected", session_id);

  // We have the return the sessions's request if it
  // has one ongoing
  const auto& session = sessions[session_id];
  if (session.request_ongoing)
  {
    LOG_INFO("Returning session's request to queue");
    request_queue.push_back(session.current_request);
  }

  // Delete the session
  sessions.erase(session_id);

  // Check for unrecoverable scenario
  if (sessions.empty() && !request_queue.empty())
  {
    LOG_ERROR("All session disconnected but there are still requests in the queue, aborting");
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief Callback called when a session has read a message
 *
 * The only message that is read is Response, so if we cannot deserialize
 * as a Response then abort.
 *
 * Otherwise add the pixels that we receive. If this is not the last message
 * then we continue to read messages. If this is the last message we continue
 * to send next Request in the queue. If the queue is empty we are done
 * and can close the session.
 *
 * @param[in]  session_id  Id of the session that has read a message
 * @param[in]  buffer      Message data
 * @param[in]  len         Length of message data
 */
static void on_read(int session_id, const std::uint8_t* buffer, int len)
{
  auto& session = sessions[session_id];

  LOG_DEBUG("%s: session_id=%d len=%d", __func__, session_id, len);

  Protocol::Response response;
  if (!Protocol::deserialize(std::vector<std::uint8_t>(buffer, buffer + len), &response))
  {
    LOG_ERROR("%s: could not deserialize Response message", __func__);
    session.connection->close();
  }

  LOG_DEBUG("%s: response: num_pixels=%d last_message=%s",
            __func__,
            static_cast<int>(response.pixels.size()),
            (response.last_message ? "true" : "false"));

  // Add the pixels we received
  session.current_pixels.insert(session.current_pixels.end(), response.pixels.begin(), response.pixels.end());

  if (response.last_message == 0)
  {
    // Read more messages
    session.connection->read();
    return;
  }

  LOG_INFO("Session %d request completed", session_id);

  // Add current_pixels to image_pixels, at the correct position
  // TODO: Simplify this, if possible
  const auto sub_c = (arguments.max_c - arguments.min_c) / static_cast<double>(arguments.divisions);
  const auto dx = ((session.current_request.min_c - arguments.min_c) / sub_c.real()).real();
  const auto dy = ((session.current_request.min_c - arguments.min_c) / sub_c.imag()).imag();
  auto to = image_pixels.begin() +
            (session.current_request.image_height * dy * arguments.image_width) +
            (session.current_request.image_width * dx);
  auto from = session.current_pixels.begin();
  for (auto y = 0u; y < session.current_request.image_height; y++)
  {
    std::copy(from, from + session.current_request.image_width, to);
    from += session.current_request.image_width;
    to += arguments.image_width;
  }

  // Current request is done
  session.request_ongoing = false;

  // Check if there are more requests to handle
  if (!request_queue.empty())
  {
    // Handle next request
    send_request(session_id);
    return;
  }

  // Check if this is the last session to finish (no Session has request_ongoing = true)
  if (std::none_of(sessions.begin(),
                   sessions.end(),
                   [](const auto& pair){ return pair.second.request_ongoing; }))
  {
    // Now we can close all sessions and end the program
    // NOTE: close() will call the on_disconnected callback which in turn will
    //       delete the Session and remove it from the sessions vector, so we
    //       cannot iterate over the sessions vector and call close()
    while (!sessions.empty())
    {
      auto& session = sessions.begin()->second;
      session.connection->close();
    }
  }
}

/**
 * @brief Callback called when a session has written a message
 *
 * Not used besides for debugging.
 *
 * @param[in]  session_id  Id of the session that has written a message
 */
static void on_write(int session_id)
{
  LOG_DEBUG("%s: session_id=%d", __func__, session_id);
}

/**
 * @brief Callback called when an error occurs in the Session connection
 *
 * Close the connection if any error occurs and let the on_disconnected
 * callback decide if we can continue or if we have to abort.
 *
 * @param[in]  session_id  Id of the session for which an error occurred
 * @param[in]  message     Error message
 */
static void on_error_connection(int session_id, const std::string& message)
{
  LOG_ERROR("%s: session_id=%d message=%s", __func__, session_id, message.c_str());
  auto& session = sessions.at(session_id);
  session.connection->close();
}

/**
 * @brief Callback called when an error occurs in TcpBackend::connect
 *
 * Just print the error and continue. It doesn't matter that one or more
 * connections fail as long as at least one connection is successful.
 * If _all_ connections fail the network backend will return and we'll notice
 * in main() that we still have unhandled requests in the queue, and inform
 * the user that the program failed.
 *
 * @param[in]  message     Error message
 */
static void on_error_client(const std::string& message)
{
  LOG_ERROR("%s: message=%s", __func__, message.c_str());
}

/**
 * @brief Callback called when TcpBackend::connect successfully connected
 *
 * Creates and initializes a Session object with the given Connection.
 *
 * @param[in]  connection  The Connection, wrapped in std::unique_ptr
 * @param[in]  address     The client address
 * @param[in]  port        The client port
 */
static void on_connected(std::unique_ptr<TcpBackend::Connection>&& connection,
                         const std::string& address,
                         const std::string& port)
{
  // One unique id per session/connection
  static int next_session_id = 0;
  const auto session_id = next_session_id;
  next_session_id += 1;

  LOG_INFO("Session %d connected to %s:%s", session_id, address.c_str(), port.c_str());

  // Create and store session object
  auto& session = sessions[session_id];
  session.connection = std::move(connection);
  session.request_ongoing = false;

  // Create callbacks
  // These are just wrappers for on_read/on_write/on_error_connection
  // but with session_id captured, so that we can differentiate the
  // session in the callback functions
  auto disconnected = [session_id]()                                    { on_disconnected(session_id);              };
  auto read         = [session_id](const std::uint8_t* buffer, int len) { on_read(session_id, buffer, len);         };
  auto write        = [session_id]()                                    { on_write(session_id);                     };
  auto error        = [session_id](const std::string& message)          { on_error_connection(session_id, message); };

  // Set callbacks and start the read procedure
  sessions[session_id].connection->set_callbacks(disconnected, read, write, error);

  // Send next request in queue to this session
  send_request(session_id);
}

/**
 * @brief Main
 *
 * Parses arguments.
 * Creates Requests.
 * Starts connections.
 * Prints timing information.
 *
 * @param[in]  argc  argc
 * @param[in]  argv  argv
 *
 * @return exit code
 */
int main(int argc, char* argv[])
{
  // Check and parse arguments
  if (argc < 10)
  {
    fprintf(stderr,
            "usage: %s min_c_re min_c_im max_c_re max_c_im max_n x y divisions list-of-servers\n",
            argv[0]);
    return EXIT_FAILURE;
  }

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
    fprintf(stderr, "exception: %s\n", e.what());
    fprintf(stderr,
            "usage: %s min_c_re min_c_im max_c_re max_c_im max_n x y divisions list-of-servers\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  std::vector<std::tuple<std::string, std::string>> servers;
  for (auto i = 9; i < argc; i++)
  {
    const auto arg = std::string(argv[i]);
    const auto sep = arg.find_last_of(":");
    if (sep == 0u ||                 // no address part
        sep == std::string::npos ||  // no colon
        sep == arg.size() - 1)       // no port part
    {
      fprintf(stderr,
              "usage: %s min_c_re min_c_im max_c_re max_c_im max_n x y divisions list-of-servers\n",
              argv[0]);
      return EXIT_FAILURE;
    }

    const auto address = arg.substr(0, sep);
    const auto port = arg.substr(sep + 1);
    servers.emplace_back(address, port);
  }

  // Split image/computation into sub-images and add a request
  // for each sub-image
  const auto sub_image_width  = arguments.image_width / arguments.divisions;
  const auto sub_image_height = arguments.image_height / arguments.divisions;
  const auto sub_image_c_step = (arguments.max_c - arguments.min_c) /
                                static_cast<double>(arguments.divisions);
  for (auto y = 0; y < arguments.divisions; y++)
  {
    for (auto x = 0; x < arguments.divisions; x++)
    {
      Protocol::Request request;
      request.min_c        = arguments.min_c + std::complex<double>(sub_image_c_step.real() * x,
                                                                    sub_image_c_step.imag() * y);
      request.max_c        = request.min_c + sub_image_c_step;
      request.image_width  = sub_image_width;
      request.image_height = sub_image_height;
      request.max_iter     = arguments.max_iter;
      request_queue.push_back(request);
    }
  }

  // Pre-allocate image_pixels vector
  image_pixels.insert(image_pixels.begin(), arguments.image_width * arguments.image_height, 0u);

  // Connect towards each server
  for (const auto& server : servers)
  {
    const auto address = std::get<0>(server);
    const auto port = std::get<1>(server);
    LOG_INFO("Creating TCP client connecting to %s:%s", address.c_str(), port.c_str());
    TcpBackend::connect(address, port, on_connected, on_error_client);
  }

  // Save timestamp at start
  const auto time_begin = std::chrono::steady_clock::now();

  // Start network backend, it will run until there are no more
  // active async tasks
  TcpBackend::run();

  // Check if network backend returned prematurely
  if (!sessions.empty() || !request_queue.empty())
  {
    LOG_ERROR("%s: network backend return but there are still sessions or requests in the queue",
              __func__);
    exit(EXIT_FAILURE);
  }

  // Write image file
  static const auto filename = std::string("image.pgm");
  LOG_INFO("Writing image to \"%s\"", filename.c_str());
  PGM::write_pgm(filename, arguments.image_width, arguments.image_height, image_pixels.data());

  // Save timestamp at end and print execution time
  const auto time_end = std::chrono::steady_clock::now();
  LOG_INFO("%s executed for a total time of %dms (%ds)",
           argv[0],
           std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_begin).count(),
           std::chrono::duration_cast<std::chrono::seconds>(time_end - time_begin).count());

  return EXIT_SUCCESS;
}
