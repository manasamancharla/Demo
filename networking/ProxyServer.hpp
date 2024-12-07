#ifndef PROXYSERVER_HPP
#define PROXYSERVER_HPP

#define ASIO_STANDALONE

#include <asio.hpp>
#include <thread>
#include "LRU.hpp"

class ProxyServer
{
public:
  ProxyServer(asio::io_context &io_context, unsigned short port, size_t threadPoolSize, size_t cacheCapacity)
      : io_context_(io_context),
        acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
        threadPoolSize_(threadPoolSize),
        cache_(cacheCapacity) {}

  void start();

private:
  void acceptConnections();
  void handleClient(std::shared_ptr<asio::ip::tcp::socket> clientSocket);
  void forwardRequestToServer(std::shared_ptr<asio::ip::tcp::socket> clientSocket, const std::string &request, const std::string &cacheKey);
  void readFromServer(std::shared_ptr<asio::ip::tcp::socket> clientSocket, std::shared_ptr<asio::ip::tcp::socket> serverSocket, std::shared_ptr<std::vector<char>> buffer, const std::string &cacheKey);
  std::string extractCacheKey(const std::string &request);

  asio::io_context &io_context_;
  asio::ip::tcp::acceptor acceptor_;
  size_t threadPoolSize_;
  LRUCache cache_;
};

#endif
