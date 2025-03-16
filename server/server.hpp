#pragma once

#include <asio.hpp>
#include "session.hpp"

class Session;

class Server {
public:
  Server(asio::io_context& io, const asio::ip::tcp::endpoint& endpoint);

private:
  void start_accept();
  void handle_accept(Session::pointer session, const std::error_code& e);

  asio::ip::tcp::acceptor acceptor_;
  asio::io_context& io_context_;

  Lobby lobby_;
};