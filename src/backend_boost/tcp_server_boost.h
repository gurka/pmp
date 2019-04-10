#ifndef TCP_SERVER_BOOST_H_
#define TCP_SERVER_BOOST_H_

#include "tcp_backend.h"

#include <cstdint>

#include <boost/asio.hpp>

namespace TcpBackend
{

class ServerBoost : public Server
{
 public:
  ServerBoost(boost::asio::io_service* io_service,
              std::uint16_t port,
              const OnAccept& on_accept);

 private:
  void async_accept();

  boost::asio::ip::tcp::acceptor m_acceptor;
  boost::asio::ip::tcp::socket m_socket;
  OnAccept m_on_accept;
};

}

#endif  // TCP_SERVER_BOOST_H_
