#ifndef PROXY_SERVER_HPP
#define PROXY_SERVER_HPP

#define ASIO_STANDALONE

#include <asio.hpp>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <iostream>

class ProxyServer
{
public:
  ProxyServer(unsigned short port);
  void start();
  void stop();

private:
  void acceptConnection();
  void handleRequest(std::shared_ptr<asio::ip::tcp::socket> clientSocket);
  void forwardRequestToServer(std::shared_ptr<asio::ip::tcp::socket> clientSocket, const std::string &request);
  void readFromServer(std::shared_ptr<asio::ip::tcp::socket> clientSocket,
                      std::shared_ptr<asio::ip::tcp::socket> serverSocket,
                      std::shared_ptr<std::vector<char>> buffer);
  std::string extractHostFromRequest(const std::string &request);

  asio::io_context io_context_;
  asio::ip::tcp::acceptor acceptor_;
  std::thread serverThread_;
};

#endif
