#pragma once

#include <asio.hpp>
#include <string>
#include <memory>
#include <array>

std::string extract_host_from_request(const std::string &request);

class ProxySession : public std::enable_shared_from_this<ProxySession>
{
public:
  ProxySession(asio::ip::tcp::socket client_socket, asio::io_context &io_context);
  void start();

private:
  void handle_client_request(std::size_t length);
  void connect_to_server(asio::ip::tcp::resolver::results_type &endpoints, const std::string &request);
  void forward_request_to_server(const std::string &request);
  void read_server_response();
  void forward_response_to_client(std::size_t length);

  asio::ip::tcp::socket client_socket_;
  asio::ip::tcp::socket server_socket_;
  asio::ip::tcp::resolver resolver_;
  std::array<char, 1024> buffer_;
};
