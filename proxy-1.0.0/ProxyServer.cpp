#include "ProxyServer.hpp"
#include "ProxySession.hpp"
#include <iostream>

ProxyServer::ProxyServer(asio::io_context &io_context, unsigned short port)
    : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), io_context_(io_context)
{
  start_accepting();
}

void ProxyServer::start_accepting()
{
  acceptor_.async_accept(
      [this](std::error_code ec, asio::ip::tcp::socket client_socket)
      {
        if (!ec)
        {
          std::cout << "Accepted a connection!" << std::endl;
          std::make_shared<ProxySession>(std::move(client_socket), io_context_)->start();
        }
        start_accepting(); // Continue accepting new connections
      });
}
