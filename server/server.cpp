#include "server.hpp"
#include <iostream>

Server::Server(asio::io_context &io, const asio::ip::tcp::endpoint &endpoint)
    : io_context_(io), acceptor_(io, endpoint) {
  start_accept();
}

void Server::start_accept() {
  Session::pointer new_session = Session::create(io_context_, &lobby_);
  acceptor_.async_accept(new_session->socket(),
                         std::bind(&Server::handle_accept, this,
                                   new_session, asio::placeholders::error));
}

void Server::handle_accept(Session::pointer session, const std::error_code &e) {
  LOG_DEBUG("Server::handle_accept");
  if (!e) {
    session->start();
  }

  start_accept();
}
