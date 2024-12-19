#include <iostream>
#include <asio.hpp>
#include "ProxyServer.hpp"

int main()
{
  try
  {
    asio::io_context io_context;
    ProxyServer server(io_context, 8080);

    std::cout << "Proxy server running on port 8080..." << std::endl;
    io_context.run();
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
