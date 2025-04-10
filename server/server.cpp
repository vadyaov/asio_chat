#include "server.hpp"
#include "room-mgr.h"
#include "session.hpp"
#include <system_error>

Server::Server(asio::io_context &io, const asio::ip::tcp::endpoint &endpoint)
    : io_context_(io), acceptor_(io, endpoint), room_mgr_(new RoomMgr) {
  start_accept();
}

void Server::start_accept() {
  Session::pointer new_session = Session::create(io_context_, room_mgr_.get());
  acceptor_.async_accept(new_session->socket(), [this, new_session](const std::error_code& ec) -> void {
    if (!ec) {
      new_session->start();
    }
    start_accept();
  });
}