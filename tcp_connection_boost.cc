#include "tcp_connection_boost.h"

TcpConnectionBoost::TcpConnectionBoost(boost::asio::ip::tcp::socket socket)
    : m_socket(std::move(socket))
{
}

void TcpConnectionBoost::close()
{
  m_socket.close();
}
