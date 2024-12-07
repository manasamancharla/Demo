#define ASIO_STANDALONE

#include <iostream>
#include <asio.hpp>
#include "networking/ProxyServer.hpp"

int main(int argc, char *argv[])
{
  // Set default values for port, thread pool size, and cache capacity
  unsigned short port = 8080;
  size_t threadPoolSize = 4;  // Adjust based on your system or preference
  size_t cacheCapacity = 100; // Set cache capacity (you can change this as needed)

  // Create the io_context required for Asio operations
  asio::io_context io_context;

  // Initialize the ProxyServer with the io_context and the parameters
  ProxyServer server(io_context, port, threadPoolSize, cacheCapacity);

  std::cout << "Proxy server running on port " << port << "..." << std::endl;

  // Start the server
  server.start();

  return 0;
}