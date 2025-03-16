#include "session.hpp"
#include "message_parser.hpp"
#include <system_error>

void Session::start() {
  LOG_DEBUG("Session::start");
  current_room_->join(shared_from_this());
  do_read_header();
}

void Session::deliver(const server_message &msg) {
  LOG_DEBUG("Session::deliver");
  bool write_in_progress = !write_msgs_.empty();
  write_msgs_.push_back(msg);
  if (!write_in_progress) {
    do_write_header();
  }
}

void Session::leaveCurrentRoom() {
  if (current_room_)
    current_room_->leave(shared_from_this());
}

void Session::toRoom(IRoom *new_room) {
  leaveCurrentRoom();
  current_room_ = new_room;
  current_room_->join(shared_from_this());
}

void Session::toLobby() {
  leaveCurrentRoom();
  current_room_ = nullptr;
  current_room_->join(shared_from_this());
}

void Session::disconnect() {
  asio::error_code ec;
  socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
  socket_.close(ec);
}

Session::Session(asio::io_context &io, IRoom *room)
    : socket_(io), lobby_(room), current_room_(room) {}

void Session::do_read_header() {
  asio::async_read(
      socket_, asio::buffer(&read_message_.header, sizeof(chat_header)),
      [this](const std::error_code &ec, size_t /* bytes_transferred */) {
        if (!ec /* && isCorrectHeader */) {
          do_read_body();
        } else {
          LOG_DEBUG(ec.message());
          current_room_->leave(shared_from_this());
        }
      });
}

void Session::do_read_body() {
  read_message_.body.resize(read_message_.header.size);
  asio::async_read(
      socket_,
      asio::buffer(read_message_.body.data(), read_message_.header.size),
      [this](const std::error_code &ec, size_t /* bytes_transferred */) {
        if (!ec) {
          LOG_DEBUG(read_message_);
          ClientMessageParser::parse(read_message_);
          current_room_->onMessageReceived(shared_from_this(), read_message_);
          read_message_.offset = 0;
          do_read_header();
        } else {
          LOG_DEBUG(ec.message());
          current_room_->leave(shared_from_this());
        }
      });
}

void Session::do_write_header() {
  auto &front_message = write_msgs_.front();
  asio::async_write(
      socket_, asio::buffer(&front_message.header, sizeof(chat_header)),
      [this](const std::error_code &ec, size_t /* bytes_transferred */) {
        if (!ec) {
          do_write_body();
        } else {
          LOG_DEBUG(ec.message());
          current_room_->leave(shared_from_this());
        }
      });
}

void Session::do_write_body() {
  auto &front_message = write_msgs_.front();
  asio::async_write(
      socket_,
      asio::buffer(front_message.body.data(), front_message.header.size),
      [this](const std::error_code &ec, size_t /* bytes_transferred */) {
        if (!ec) {
          write_msgs_.pop_front();
          if (!write_msgs_.empty()) {
            do_write_header();
          }
        } else {
          LOG_DEBUG(ec.message());
          current_room_->leave(shared_from_this());
        }
      });
}