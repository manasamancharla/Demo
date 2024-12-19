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

// #define ASIO_STANDALONE

// #include <iostream>
// #include <asio.hpp>
// #include <string>
// #include <memory>
// #include <regex>

// using asio::ip::tcp;

// std::string extract_host_from_request(const std::string &request)
// {
//   std::regex host_regex(R"(Host:\s*([^\r\n]+))");
//   std::smatch match;
//   if (std::regex_search(request, match, host_regex))
//   {
//     return match[1];
//   }
//   return "";
// }

// class ProxySession : public std::enable_shared_from_this<ProxySession>
// {
// public:
//   ProxySession(tcp::socket client_socket, asio::io_context &io_context)
//       : client_socket_(std::move(client_socket)), server_socket_(io_context), resolver_(io_context)
//   {
//   }

//   void start()
//   {
//     auto self(shared_from_this());
//     client_socket_.async_read_some(
//         asio::buffer(buffer_),
//         [this, self](std::error_code ec, std::size_t length)
//         {
//           if (!ec)
//           {
//             handle_client_request(length);
//           }
//         });
//   }

// private:
//   void handle_client_request(std::size_t length)
//   {
//     std::string request(buffer_.data(), length);
//     std::cout << "Request from client: \n"
//               << request << std::endl;

//     // Extract the host
//     std::string host = extract_host_from_request(request);
//     if (host.empty())
//     {
//       std::cerr << "Host not found in request" << std::endl;
//       return;
//     }

//     std::cout << "Extracted host: " << host << std::endl;

//     auto self(shared_from_this());
//     resolver_.async_resolve(
//         host, "80",
//         [this, self, request](std::error_code ec, tcp::resolver::results_type endpoints)
//         {
//           if (!ec)
//           {
//             connect_to_server(endpoints, request);
//           }
//         });
//   }

//   void connect_to_server(tcp::resolver::results_type &endpoints, const std::string &request)
//   {
//     auto self(shared_from_this());
//     asio::async_connect(
//         server_socket_, endpoints,
//         [this, self, request](std::error_code ec, const tcp::endpoint &)
//         {
//           if (!ec)
//           {
//             forward_request_to_server(request);
//           }
//         });
//   }

//   void forward_request_to_server(const std::string &request)
//   {
//     auto self(shared_from_this());
//     asio::async_write(
//         server_socket_, asio::buffer(request),
//         [this, self](std::error_code ec, std::size_t)
//         {
//           if (!ec)
//           {
//             read_server_response();
//           }
//         });
//   }

//   void read_server_response()
//   {
//     auto self(shared_from_this());
//     server_socket_.async_read_some(
//         asio::buffer(buffer_),
//         [this, self](std::error_code ec, std::size_t length)
//         {
//           if (!ec)
//           {
//             forward_response_to_client(length);
//           }
//           else if (ec == asio::error::eof)
//           {
//             // EOF reached, stop reading and close the connections
//             std::cout << "EOF reached, closing connection." << std::endl;
//             client_socket_.close();
//             server_socket_.close();
//           }
//           else
//           {
//             // Other errors (e.g., network issues), handle them
//             std::cerr << "Error reading from server: " << ec.message() << std::endl;
//             client_socket_.close();
//             server_socket_.close();
//           }
//         });
//   }

//   void forward_response_to_client(std::size_t length)
//   {
//     auto self(shared_from_this());
//     asio::async_write(
//         client_socket_, asio::buffer(buffer_, length),
//         [this, self](std::error_code ec, std::size_t)
//         {
//           if (!ec)
//           {
//             read_server_response(); // Continue reading server response
//           }

//           else if (ec == asio::error::eof)
//           {
//             // EOF reached during writing, stop the process and close the connections
//             std::cout << "EOF reached, closing connection." << std::endl;
//             client_socket_.close();
//             server_socket_.close();
//           }
//           else
//           {
//             // Other errors during write, handle them
//             std::cerr << "Error forwarding response to client: " << ec.message() << std::endl;
//             client_socket_.close();
//             server_socket_.close();
//           }
//         });
//   }

//   tcp::socket client_socket_;
//   tcp::socket server_socket_;
//   tcp::resolver resolver_;
//   std::array<char, 1024> buffer_;
// };

// class ProxyServer
// {
// public:
//   ProxyServer(asio::io_context &io_context, unsigned short port)
//       : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), io_context_(io_context)
//   {
//     start_accepting();
//   }

// private:
//   void start_accepting()
//   {
//     acceptor_.async_accept(
//         [this](std::error_code ec, tcp::socket client_socket)
//         {
//           if (!ec)
//           {
//             std::cout << "Accepted a connection!" << std::endl;
//             std::make_shared<ProxySession>(std::move(client_socket), io_context_)->start();
//           }
//           start_accepting(); // Continue accepting new connections
//         });
//   }

//   tcp::acceptor acceptor_;
//   asio::io_context &io_context_;
// };

// int main()
// {
//   try
//   {
//     asio::io_context io_context;
//     ProxyServer server(io_context, 8080);

//     std::cout << "Proxy server running on port 8080..." << std::endl;
//     io_context.run();
//   }
//   catch (const std::exception &e)
//   {
//     std::cerr << "Exception: " << e.what() << std::endl;
//   }

//   return 0;
// }
