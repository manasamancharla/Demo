#define ASIO_STANDALONE

#include <iostream>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

std::vector<char> vBuffer(20 * 1024);

void getData(asio::ip::tcp::socket &socket)
{
  socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()), [&](std::error_code ec, std::size_t length)
                         {
                           if (!ec)
                           {
                             std::cout << "\n\nRead " << length << " bytes\n\n";

                             for (int i = 0; i < length; i++)
                             {
                               std::cout << vBuffer[i];
                             }

                             // Call recursively to continue reading data
                             getData(socket);
                           }
                           else if (ec != asio::error::eof)
                           {
                             std::cout << "Error during async read: " << ec.message() << std::endl;
                           } });
}

int main()
{
  asio::error_code ec;

  // Create io_context to manage I/O
  asio::io_context io_context;

  // Keep io_context running
  asio::io_context::work idleWork(io_context);

  // Start the io_context on a separate thread
  std::thread contextThread = std::thread([&]()
                                          { io_context.run(); });

  // Define the endpoint using the server's IP address and port
  asio::ip::tcp::endpoint endpoint(asio::ip::make_address("93.184.216.34", ec), 80);

  // Create a socket
  asio::ip::tcp::socket socket(io_context);

  // Attempt to connect to the server
  socket.connect(endpoint, ec);

  if (!ec)
  {
    std::cout << "Connected" << std::endl;
  }
  else
  {
    std::cout << "Failed to connect to address:\n"
              << ec.message() << std::endl;
  }

  if (socket.is_open())
  {
    // Start asynchronous reading
    getData(socket);

    std::string sRequest = "GET / HTTP/1.1\r\n"
                           "Host: example.com\r\n"
                           "Connection: close\r\n"
                           "\r\n";

    socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

    if (!ec)
    {
      std::cout << "Request sent successfully!" << std::endl;
    }
    else
    {
      std::cout << "Error sending request: " << ec.message() << std::endl;
    }

    std::cin.get();

    // Clean up
    socket.close();
    io_context.stop();
    if (contextThread.joinable())
    {
      contextThread.join();
    }
  }

  return 0;
}