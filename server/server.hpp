#pragma once

#include <asio.hpp>
#include "session.hpp"

// class ICommandHandler {
// public:
//   virtual ~ICommandHandler() {}

//   virtual void process(chat_session::pointer session, const chat_message& msg);
// };

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