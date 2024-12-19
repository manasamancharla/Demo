#pragma once

#include <asio.hpp>
#include <memory>

class ProxyServer
{
public:
  ProxyServer(asio::io_context &io_context, unsigned short port);

private:
  void start_accepting();

  asio::ip::tcp::acceptor acceptor_;
  asio::io_context &io_context_;
};
