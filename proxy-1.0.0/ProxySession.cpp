#include "ProxySession.hpp"
#include <iostream>
#include <regex>

std::string extract_host_from_request(const std::string &request)
{
  std::regex host_regex(R"(Host:\s*([^\r\n]+))");
  std::smatch match;
  if (std::regex_search(request, match, host_regex))
  {
    return match[1];
  }
  return "";
}

ProxySession::ProxySession(asio::ip::tcp::socket client_socket, asio::io_context &io_context)
    : client_socket_(std::move(client_socket)), server_socket_(io_context), resolver_(io_context)
{
}

void ProxySession::start()
{
  auto self(shared_from_this());
  client_socket_.async_read_some(
      asio::buffer(buffer_),
      [this, self](std::error_code ec, std::size_t length)
      {
        if (!ec)
        {
          handle_client_request(length);
        }
      });
}

void ProxySession::handle_client_request(std::size_t length)
{
  std::string request(buffer_.data(), length);
  std::cout << "Request from client: " << "\n"
            << request << std::endl;

  std::string host = extract_host_from_request(request);
  if (host.empty())
  {
    std::cerr << "Host not found in request" << std::endl;
    return;
  }

  std::cout << "Extracted host: " << host << std::endl;

  auto self(shared_from_this());
  resolver_.async_resolve(
      host, "80",
      [this, self, request](std::error_code ec, asio::ip::tcp::resolver::results_type endpoints)
      {
        if (!ec)
        {
          connect_to_server(endpoints, request);
        }
      });
}

void ProxySession::connect_to_server(asio::ip::tcp::resolver::results_type &endpoints, const std::string &request)
{
  auto self(shared_from_this());
  asio::async_connect(
      server_socket_, endpoints,
      [this, self, request](std::error_code ec, const asio::ip::tcp::endpoint &)
      {
        if (!ec)
        {
          forward_request_to_server(request);
        }
      });
}

void ProxySession::forward_request_to_server(const std::string &request)
{
  auto self(shared_from_this());
  asio::async_write(
      server_socket_, asio::buffer(request),
      [this, self](std::error_code ec, std::size_t)
      {
        if (!ec)
        {
          read_server_response();
        }
      });
}

void ProxySession::read_server_response()
{
  auto self(shared_from_this());
  server_socket_.async_read_some(
      asio::buffer(buffer_),
      [this, self](std::error_code ec, std::size_t length)
      {
        if (!ec)
        {
          forward_response_to_client(length);
        }
        else if (ec == asio::error::eof)
        {
          std::cout << "EOF reached, closing connection." << std::endl;
          client_socket_.close();
          server_socket_.close();
        }
        else
        {
          std::cerr << "Error reading from server: " << ec.message() << std::endl;
          client_socket_.close();
          server_socket_.close();
        }
      });
}

void ProxySession::forward_response_to_client(std::size_t length)
{
  auto self(shared_from_this());
  asio::async_write(
      client_socket_, asio::buffer(buffer_, length),
      [this, self](std::error_code ec, std::size_t)
      {
        if (!ec)
        {
          read_server_response(); // Continue reading server response
        }
        else if (ec == asio::error::eof)
        {
          std::cout << "EOF reached, closing connection." << std::endl;
          client_socket_.close();
          server_socket_.close();
        }
        else
        {
          std::cerr << "Error forwarding response to client: " << ec.message() << std::endl;
          client_socket_.close();
          server_socket_.close();
        }
      });
}
