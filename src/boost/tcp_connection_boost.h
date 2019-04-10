#ifndef TCP_CONNECTION_BOOST_H_
#define TCP_CONNECTION_BOOST_H_

#include "tcp_connection.h"

#include <cstdint>
#include <array>

#include <boost/asio.hpp>

class TcpConnectionBoost : public TcpConnection
{
 public:
  TcpConnectionBoost(boost::asio::ip::tcp::socket socket);

  void start(const OnRead& on_read,
             const OnWrite& on_write,
             const OnError& on_error) override;
  void write(const std::uint8_t* buffer, int len) override;
  void close() override;

 private:
  void async_read_header();
  void async_read_data(int data_len);

  boost::asio::ip::tcp::socket m_socket;

  // A message consists of a header and data
  // The header is 2 bytes and the value is the data length
  //
  // Example: a message with 6 bytes of data has a
  //          header that is 0x06 0x00 (network byte order)
  //          the total message length is 2 + 6 = 8 bytes
  //
  // The longest message we can send has 2^16 - 1 bytes of
  // data (the header would be 0xff 0xff) and a total length
  // of 2^16 - 1 + 2 bytes.
  //
  // Make sure that these buffers are capable of handling
  // maximum size messages. Note that the read buffer first
  // reads the header and then the message data while the
  // write buffer must be able to contain the whole message.
  // This is the reason for the size difference.
  std::array<std::uint8_t, (1 << 16) - 1> m_read_buffer;
  std::array<std::uint8_t, (1 << 16) - 1 + 2> m_write_buffer;
  bool m_write_ongoing;

  OnRead m_on_read;
  OnWrite m_on_write;
  OnError m_on_error;
};

#endif  // TCP_CONNECTION_BOOST_H_
