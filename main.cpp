#define ASIO_STANDALONE

#include <iostream>
#include <asio.hpp>
#include "networking/ProxyServer.hpp"

int main()
{
  try
  {
    unsigned short port = 8080; // Port for the proxy server to listen on

    // Create the ProxyServer instance
    ProxyServer proxyServer(port);

    // Start the server
    proxyServer.start();

    std::cout << "Proxy server running on port " << port << std::endl;

    // Wait for user input to stop the server
    std::string input;
    std::getline(std::cin, input);
    proxyServer.stop();
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
