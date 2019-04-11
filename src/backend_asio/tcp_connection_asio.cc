#include "tcp_connection_asio.h"

#include <cstdio>

namespace TcpBackend
{

ConnectionAsio::ConnectionAsio(asio::ip::tcp::socket socket)
    : m_socket(std::move(socket)),
      m_read_buffer(),
      m_write_buffer(),
      m_read_ongoing(false),
      m_write_ongoing(false),
      m_closing(false),
      m_on_disconnected(),
      m_on_read(),
      m_on_write(),
      m_on_error()
{
}

void ConnectionAsio::set_callbacks(const OnDisconnected& on_disconnected,
                                   const OnRead& on_read,
                                   const OnWrite& on_write,
                                   const OnError& on_error)
{
  m_on_disconnected = on_disconnected;
  m_on_read         = on_read;
  m_on_write        = on_write;
  m_on_error        = on_error;
}

void ConnectionAsio::read()
{
  if (m_read_ongoing)
  {
    fprintf(stderr, "%s: read procedure already ongoing!\n", __func__);
    return;
  }

  // Read header (2 bytes) into m_read_buffer and then call read_data()
  m_read_ongoing = true;
  asio::async_read(m_socket,
                   asio::buffer(m_read_buffer.data(), 2),
                   [this](const std::error_code& ec, std::size_t len)
                   {
                     m_read_ongoing = false;

                     // Check if the user or the remote side wants to close to connection
                     if (m_closing || ec == asio::error::eof)
                     {
                       m_closing = true;

                       // Check if we are ready to call OnDisconnected
                       if (!m_read_ongoing && !m_write_ongoing)
                       {
                         m_on_disconnected();
                         // Warning: this instance can be deleted now
                         //          don't access any instance variables
                       }
                       return;
                     }

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
                     if (data_len == 0u)
                     {
                       m_on_error("data_len (0) is invalid");
                       return;
                     }

                     if (data_len > static_cast<int>(m_read_buffer.size()))
                     {
                       m_on_error("data_len (" + std::to_string(data_len) + ") in header too large");
                       return;
                     }

                     read_data(data_len);
                   });
}

void ConnectionAsio::write(const std::uint8_t* buffer, int len)
{
  if (m_write_ongoing)
  {
    fprintf(stderr, "%s: write procedure already ongoing!\n", __func__);
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
  m_write_ongoing = true;
  asio::async_write(m_socket,
                    asio::buffer(m_write_buffer.data(), total_len),
                    [this, total_len](const std::error_code& ec, std::size_t len)
                    {
                      m_write_ongoing = false;

                      // Check if the user or the remote side wants to close to connection
                      if (m_closing || ec == asio::error::eof)
                      {
                        m_closing = true;

                        // Check if we are ready to call OnDisconnected
                        if (!m_read_ongoing && !m_write_ongoing)
                        {
                          m_on_disconnected();
                          // Warning: this instance can be deleted now
                          //          don't access any instance variables
                        }
                        return;
                      }

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

void ConnectionAsio::close()
{
  if (!m_closing)
  {
    m_closing = true;
    m_socket.close();

    if (!m_read_ongoing && !m_write_ongoing)
    {
      m_on_disconnected();
      // Warning: this instance can be deleted now
      //          don't access any instance variables
    }
  }
}

void ConnectionAsio::read_data(int data_len)
{
  // Read data (len bytes) into m_read_buffer
  m_read_ongoing = true;
  asio::async_read(m_socket,
                   asio::buffer(m_read_buffer.data(), data_len),
                   [this, data_len](const std::error_code& ec, std::size_t len)
                   {
                     m_read_ongoing = false;

                     // Check if the user or the remote side wants to close to connection
                     if (m_closing || ec == asio::error::eof)
                     {
                       m_closing = true;

                       // Check if we are ready to call OnDisconnected
                       if (!m_read_ongoing && !m_write_ongoing)
                       {
                         m_on_disconnected();
                         // Warning: this instance can be deleted now
                         //          don't access any instance variables
                       }
                       return;
                     }

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
                     m_on_read(m_read_buffer.data(), data_len);
                   });
}

}
