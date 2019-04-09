#include "tcp_connection_boost.h"

TcpConnectionBoost::TcpConnectionBoost(boost::asio::ip::tcp::socket socket)
    : m_socket(std::move(socket)),
      m_read_buffer(),
      m_write_buffer(),
      m_write_ongoing(false),
      m_on_read(),
      m_on_write(),
      m_on_error()
{
}

void TcpConnectionBoost::start(const OnRead& on_read,
                               const OnWrite& on_write,
                               const OnError& on_error)
{
  // Save callbacks
  m_on_read   = on_read;
  m_on_write  = on_write;
  m_on_error  = on_error;

  // Start async read procedure
  async_read_header();
}

void TcpConnectionBoost::write(const std::uint8_t* buffer, int len)
{
  if (m_write_ongoing)
  {
    fprintf(stderr, "%s: write already ongoing!\n", __func__);
    return;
  }

  if (len == 0)
  {
    return;
  }

  const auto total_len = 2 + len;

  if (total_len > static_cast<int>(m_write_buffer.size()))
  {
    fprintf(stderr, "%s: trying to send too much data (%d)\n", __func__, len);
    return;
  }

  // Write header to buffer
  m_write_buffer[0] = len & 0xff;
  m_write_buffer[1] = (len >> 8) & 0xff;

  // Copy data to buffer
  std::copy(buffer, buffer + len, m_write_buffer.data() + 2);

  // Send buffer
  boost::asio::async_write(m_socket,
                           boost::asio::buffer(m_write_buffer.data(), total_len),
                           [this, total_len](const boost::system::error_code& ec, std::size_t len)
                           {
                             if (ec)
                             {
                               m_on_error(ec.message());
                               return;
                             }

                             if (total_len != static_cast<int>(len))
                             {
                               m_on_error("Could not send all data");
                               return;
                             }

                             m_on_write();
                           });
}

void TcpConnectionBoost::close()
{
  m_socket.close();
}

void TcpConnectionBoost::async_read_header()
{
  // Read header (2 bytes) into m_read_buffer
  boost::asio::async_read(m_socket,
                          boost::asio::buffer(m_read_buffer.data(), 2),
                          [this](const boost::system::error_code& ec, std::size_t len)
                          {
                            if (ec)
                            {
                              m_on_error(ec.message());
                              return;
                            }

                            if (len != 2u)
                            {
                              m_on_error("Could not read header");
                              return;
                            }

                            const auto data_len = (m_read_buffer[1] << 8) | m_read_buffer[0];
                            if (data_len > static_cast<int>(m_read_buffer.size()))
                            {
                              m_on_error("data_len (" + std::to_string(data_len) + ") in header too large");
                              return;
                            }

                            async_read_data(data_len);
                          });
}

void TcpConnectionBoost::async_read_data(int data_len)
{
  // Read data (len bytes) into m_read_buffer
  boost::asio::async_read(m_socket,
                          boost::asio::buffer(m_read_buffer.data(), data_len),
                          [this, data_len](const boost::system::error_code& ec, std::size_t len)
                          {
                            if (ec)
                            {
                              m_on_error(ec.message());
                              return;
                            }

                            if (static_cast<int>(len) != data_len)
                            {
                              m_on_error("Could not read data");
                              return;
                            }

                            // Call callback with data and data length
                            if (m_on_read(m_read_buffer.data(), data_len))
                            {
                              // Continue to read next header
                              async_read_header();
                            }
                          });
}
