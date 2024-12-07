#include "ProxyServer.hpp"
#include <iostream>

void ProxyServer::start()
{
    acceptConnections();

    std::vector<std::thread> threads;
    for (size_t i = 0; i < threadPoolSize_; ++i)
    {
        threads.emplace_back([this]()
                             { io_context_.run(); });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
}

void ProxyServer::acceptConnections()
{
    auto clientSocket = std::make_shared<asio::ip::tcp::socket>(io_context_);
    acceptor_.async_accept(*clientSocket, [this, clientSocket](std::error_code ec)
                           {
        if (!ec) {
            handleClient(clientSocket);
        } else {
            std::cerr << "Error accepting connection: " << ec.message() << std::endl;
        }
        acceptConnections(); });
}

void ProxyServer::handleClient(std::shared_ptr<asio::ip::tcp::socket> clientSocket)
{
    auto buffer = std::make_shared<std::vector<char>>(8192);
    clientSocket->async_read_some(asio::buffer(*buffer), [this, clientSocket, buffer](std::error_code ec, std::size_t length)
                                  {
        if (!ec) {
            std::string request(buffer->data(), length);
            std::string cacheKey = extractCacheKey(request);

            std::string cachedResponse = cache_.get(cacheKey);
            if (!cachedResponse.empty()) {
                asio::async_write(*clientSocket, asio::buffer(cachedResponse), [](std::error_code ec, std::size_t) {
                    if (ec) {
                        std::cerr << "Error sending cached response: " << ec.message() << std::endl;
                    }
                });
                return;
            }

            forwardRequestToServer(clientSocket, request, cacheKey);
        } else {
            std::cerr << "Error reading from client: " << ec.message() << std::endl;
            clientSocket->close();
        } });
}

void ProxyServer::forwardRequestToServer(std::shared_ptr<asio::ip::tcp::socket> clientSocket, const std::string &request, const std::string &cacheKey)
{
    auto serverSocket = std::make_shared<asio::ip::tcp::socket>(io_context_);
    asio::ip::tcp::resolver resolver(io_context_);
    auto buffer = std::make_shared<std::vector<char>>(8192);

    resolver.async_resolve("example.com", "80", [this, clientSocket, serverSocket, request, buffer, cacheKey](std::error_code ec, asio::ip::tcp::resolver::results_type results)
                           {
        if (!ec) {
            asio::async_connect(*serverSocket, results, [this, clientSocket, serverSocket, request, buffer, cacheKey](std::error_code ec, asio::ip::tcp::endpoint)
                                {
                if (!ec) {
                    asio::async_write(*serverSocket, asio::buffer(request), [this, clientSocket, serverSocket, buffer, cacheKey](std::error_code ec, std::size_t)
                                      {
                        if (!ec) {
                            readFromServer(clientSocket, serverSocket, buffer, cacheKey);
                        } else {
                            std::cerr << "Error forwarding request to server: " << ec.message() << std::endl;
                            clientSocket->close();
                        }
                    });
                } else {
                    std::cerr << "Error connecting to server: " << ec.message() << std::endl;
                    clientSocket->close();
                }
            });
        } else {
            std::cerr << "Error resolving server: " << ec.message() << std::endl;
            clientSocket->close();
        } });
}

void ProxyServer::readFromServer(std::shared_ptr<asio::ip::tcp::socket> clientSocket, std::shared_ptr<asio::ip::tcp::socket> serverSocket, std::shared_ptr<std::vector<char>> buffer, const std::string &cacheKey)
{
    serverSocket->async_read_some(asio::buffer(*buffer), [this, clientSocket, serverSocket, buffer, cacheKey](std::error_code ec, std::size_t length)
                                  {
        if (!ec) {
            std::string response(buffer->data(), length);
            cache_.put(cacheKey, response);

            asio::async_write(*clientSocket, asio::buffer(response), [this, clientSocket](std::error_code ec, std::size_t)
                              {
                if (ec) {
                    std::cerr << "Error forwarding response to client: " << ec.message() << std::endl;
                }
            });
        } else {
            std::cerr << "Error reading from server: " << ec.message() << std::endl;
            clientSocket->close();
        } });
}

std::string ProxyServer::extractCacheKey(const std::string &request)
{
    // Simplistic key extraction, e.g., extract the URL from the request
    size_t pos = request.find(" ");
    if (pos != std::string::npos)
    {
        size_t endPos = request.find(" ", pos + 1);
        return request.substr(pos + 1, endPos - pos - 1);
    }
    return request;
}