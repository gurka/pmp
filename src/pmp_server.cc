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

/**
 * Represents a session
 */
struct Session
{
  std::unique_ptr<TcpBackend::Connection> connection;  /**< Pointer to Connection */
  std::vector<std::uint8_t> pixels;                    /**< Array of pixels to be sent
                                                            If Request not yet received or
                                                            all Responses already sent it
                                                            will be empty */
};

// Map session_id -> Session
static std::unordered_map<int, Session> sessions;

/**
 * @brief Send (or continue to send) Response to given Session
 *
 * Takes as many pixels as possible (Response message has a max size)
 * from responses and sends it to the Session in a Response message.
 *
 * @param[in]  session_id  Id of the session to which send Response
 */
static void send_response(int session_id)
{
  // Get Session and verify that we have pixels to send
  auto& session = sessions.at(session_id);
  if (session.pixels.empty())
  {
    LOG_ERROR("%s: session_id=%d: no pixels to send", __func__, session_id);
    return;
  }

  // Maximum size of byte array (pixels) in messages is 2^15 = 32768
  // so we need to split the response into multiple messages if we have
  // more pixels than that
  Protocol::Response response;
  if (session.pixels.size() <= (1u << 15))
  {
    // We can send all pixels in this message
    // Serialize and send the message
    response.pixels = std::move(session.pixels);
    response.last_message = true;
    const auto buffer = Protocol::serialize(response);
    session.connection->write(buffer.data(), buffer.size());

    // Make sure that the Session's pixels vector is cleared
    // (moving from it leaves it in an unspecified but defined state)
    session.pixels.clear();
  }
  else
  {
    // Move as many pixels as possible (2^15) from Session's pixels vector
    // to Response's pixels vector
    response.pixels.insert(response.pixels.end(),
                           session.pixels.begin(),
                           session.pixels.begin() + (1u << 15));
    session.pixels.erase(session.pixels.begin(), session.pixels.begin() + (1u << 15));

    // Send the Response
    response.last_message = false;
    const auto buffer = Protocol::serialize(response);
    session.connection->write(buffer.data(), buffer.size());
  }
}

/**
 * @brief Callback called when a session disconnects
 *
 * Session is deleted. Server doesn't care if the disconnect
 * was expected or not.
 *
 * @param[in]  session_id  Id of the session that disconnected
 */
static void on_disconnected(int session_id)
{
  LOG_INFO("Session %d disconnected", session_id);
  sessions.erase(session_id);
}

/**
 * @brief Callback called when a session has read a message
 *
 * The only message that is read is Request, so if we cannot deserialize
 * as a Request then we close the session.
 *
 * The Mandelbrot pixels are computated and added to session's pixels vector
 * and we then start sending a response to the session.
 *
 * @param[in]  session_id  Id of the session that has read a message
 * @param[in]  buffer      Message data
 * @param[in]  len         Length of message data
 */
static void on_read(int session_id, const std::uint8_t* buffer, int len)
{
  auto& session = sessions.at(session_id);

  Protocol::Request request;
  if (!Protocol::deserialize(std::vector<std::uint8_t>(buffer, buffer + len), &request))
  {
    LOG_ERROR("%s: session_id=%d: could not deseralize Request message, closing session",
              __func__,
              session_id);
    session.connection->close();
    return;
  }

  LOG_INFO("Received request from session %d: (%.2lf, %.2lf)..(%.2lf, %.2lf) (%d, %d) %d",
           session_id,
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

  LOG_INFO("The request from session %d took %dms (%ds) to compute",
           session_id,
           std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_begin).count(),
           std::chrono::duration_cast<std::chrono::seconds>(time_end - time_begin).count());

  // Add (move) the pixels to the session object and start sending a response
  session.pixels = std::move(pixels);
  send_response(session_id);
}

/**
 * @brief Callback called when a session has written a message
 *
 * If the session has more pixels to send we do that, otherwise
 * we re-start the read procedure to see if the client has more
 * requests.
 *
 * @param[in]  session_id  Id of the session that has written a message
 */
static void on_write(int session_id)
{
  LOG_DEBUG("%s: session_id=%d", __func__, session_id);

  // Check if there are more pixels to send
  auto& session = sessions.at(session_id);
  if (!session.pixels.empty())
  {
    send_response(session_id);
  }
  else
  {
    LOG_INFO("Response successfully sent to session %d", session_id);

    // Restart read procedure to see if the client has more requests
    session.connection->read();
  }
}

/**
 * @brief Callback called when an error occurs in a session
 *
 * The session is closed on any type of error.
 *
 * @param[in]  session_id  Id of the session for which an error occurred
 * @param[in]  message     Error message
 */
static void on_error(int session_id, const std::string& message)
{
  LOG_ERROR("%s: session_id=%d message=%s", __func__, session_id, message.c_str());
  auto& session = sessions.at(session_id);
  session.connection->close();
}

/**
 * @brief Callback called when the Server has accepted a connection
 *
 * A Session is created and the read procedure is started.
 * We also tell the server to continue accepting new connections.
 *
 * @param[in]  connection  The connection, wrapped in std::unique_ptr
 */
static void on_accept(std::unique_ptr<TcpBackend::Connection>&& connection)
{
  // Unique id per session / connection
  static int next_session_id = 0;
  const auto session_id = next_session_id++;

  LOG_INFO("Session %d connected", session_id);

  // Create Session
  auto& session = sessions[session_id];
  session.connection = std::move(connection);

  // Set callbacks
  const auto disconnected  = [session_id]()                                    { on_disconnected(session_id);      };
  const auto read          = [session_id](const std::uint8_t* buffer, int len) { on_read(session_id, buffer, len); };
  const auto write         = [session_id]()                                    { on_write(session_id);             };
  const auto error         = [session_id](const std::string& message)          { on_error(session_id, message);    };
  session.connection->set_callbacks(disconnected, read, write, error);

  // Start read procedure
  session.connection->read();

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
