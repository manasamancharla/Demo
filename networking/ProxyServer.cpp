#define ASIO_STANDALONE

#include "ProxyServer.hpp"
#include <iostream>
#include <asio.hpp>

ProxyServer::ProxyServer(unsigned short port)
    : acceptor_(io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

void ProxyServer::start()
{
    acceptConnection();
    serverThread_ = std::thread([this]()
                                { io_context_.run(); });
}

void ProxyServer::stop()
{
    io_context_.stop();
    if (serverThread_.joinable())
    {
        serverThread_.join();
    }
}

void ProxyServer::acceptConnection()
{

    auto clientSocket = std::make_shared<asio::ip::tcp::socket>(io_context_);
    acceptor_.async_accept(*clientSocket, [this, clientSocket](std::error_code ec)
                           {
                               if (!ec)
                               {
                                   std::cout << "Accepted a connection!" << std::endl;
                                   handleRequest(clientSocket);
                               }
                               else
                               {
                                   std::cerr << "Error accepting connection: " << ec.message() << std::endl;
                               }
                               acceptConnection(); // Accept next connection
                           });
}

void ProxyServer::handleRequest(std::shared_ptr<asio::ip::tcp::socket> clientSocket)
{
    auto buffer = std::make_shared<std::vector<char>>(8192);
    clientSocket->async_read_some(asio::buffer(*buffer), [this, clientSocket, buffer](std::error_code ec, std::size_t length)
                                  {
        if (!ec) {
            std::string request(buffer->begin(), buffer->begin() + length);
            std::string host = extractHostFromRequest(request);
            std::cout << "request"<< request << std::endl;


            forwardRequestToServer(clientSocket, request);
        } else {
            std::cerr << "Error reading request: " << ec.message() << std::endl;
        } });
}

void ProxyServer::forwardRequestToServer(std::shared_ptr<asio::ip::tcp::socket> clientSocket, const std::string &request)
{
    std::string host = extractHostFromRequest(request);
    auto serverSocket = std::make_shared<asio::ip::tcp::socket>(io_context_);
    asio::ip::tcp::resolver resolver(io_context_);
    auto buffer = std::make_shared<std::vector<char>>(8192);

    std::cout << "Extracted host: " << host << std::endl;

    if (host.empty())
    {
        std::cerr << "Host not found in request" << std::endl;
        clientSocket->close();
        return;
    }

    resolver.async_resolve(host, "http", [this, clientSocket, serverSocket, request, buffer](std::error_code ec, asio::ip::tcp::resolver::results_type results)
                           {
        if (!ec) {
            std::cout << "Resolver success, connecting to server..." << std::endl;
            asio::async_connect(*serverSocket, results, [this, clientSocket, serverSocket, request, buffer](std::error_code ec, asio::ip::tcp::endpoint) {
                if (!ec) {
                    asio::async_write(*serverSocket, asio::buffer(request), [this, clientSocket, serverSocket, buffer](std::error_code ec, std::size_t) {
                        if (!ec) {
                            readFromServer(clientSocket, serverSocket, buffer);
                        } else {
                            std::cerr << "Error forwarding request: " << ec.message() << std::endl;
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

void ProxyServer::readFromServer(std::shared_ptr<asio::ip::tcp::socket> clientSocket,
                                 std::shared_ptr<asio::ip::tcp::socket> serverSocket,
                                 std::shared_ptr<std::vector<char>> buffer)
{
    serverSocket->async_read_some(asio::buffer(*buffer), [this, clientSocket, serverSocket, buffer](std::error_code ec, std::size_t length)
                                  {
        if (!ec) {
            asio::async_write(*clientSocket, asio::buffer(*buffer, length), [this, clientSocket, serverSocket, buffer](std::error_code ec, std::size_t) {
                if (!ec) {
                    readFromServer(clientSocket, serverSocket, buffer);
                } else {
                    std::cerr << "Error writing to client: " << ec.message() << std::endl;
                    serverSocket->close();
                }
            });
        } else {
            std::cerr << "Error reading from server: " << ec.message() << std::endl;
            serverSocket->close();
        } });
}

std::string ProxyServer::extractHostFromRequest(const std::string &request)
{
    size_t hostPos = request.find("Host: ");
    if (hostPos != std::string::npos)
    {
        size_t endPos = request.find("\r\n", hostPos + 6);
        return request.substr(hostPos + 6, endPos - (hostPos + 6));
    }
    return "";
}
