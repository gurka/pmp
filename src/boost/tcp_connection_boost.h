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

  std::array<std::uint8_t, 1024> m_read_buffer;
  std::array<std::uint8_t, 1024> m_write_buffer;
  bool m_write_ongoing;

  OnRead m_on_read;
  OnWrite m_on_write;
  OnError m_on_error;
};

#endif  // TCP_CONNECTION_BOOST_H_
