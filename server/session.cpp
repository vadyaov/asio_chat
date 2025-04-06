#include "session.hpp"
#include <system_error>
#include <iostream>

void Session::start() {
  lobby_->join(shared_from_this());
  do_read_header();
}

void Session::deliver(const server_message &msg) {
  bool write_in_progress = !write_msgs_.empty();
  write_msgs_.push_back(msg);
  if (!write_in_progress) {
    do_write_header();
  }
}

void Session::toRoom(IRoom *new_room) {
  // std::cout << "Session::toRoom" << std::endl;
  current_room_ = new_room;
  current_room_->join(shared_from_this());
}

void Session::toLobby() { current_room_ = nullptr; }

void Session::disconnect() {
  // std::cout << "Session::disconnect" << std::endl;
  // asio::error_code ec;
  // socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
  // socket_.close(ec);
}

Session::Session(asio::io_context &io, IRoom *room)
    : socket_(io), lobby_(room), current_room_(nullptr) {}

void Session::do_read_header() {
  // std::cout << "Session::do_read_header: " << std::endl;
  // std::cout << read_message_ << std::endl;
  asio::async_read(
      socket_, asio::buffer(&read_message_.header, sizeof(chat_header)),
      [this](const std::error_code &ec, size_t /* bytes_transferred */) {
        if (!ec) {
          do_read_body();
        } else {
          std::cout << "Error read header" << std::endl;
        }
      });
}

void Session::do_read_body() {
  // std::cout << "Session::do_read_body:" << std::endl;
  // std::cout << read_message_ << std::endl;
  read_message_.body.clear();
  read_message_.body.resize(read_message_.header.size);
  asio::async_read(
      socket_,
      asio::buffer(read_message_.body.data(), read_message_.header.size),
      [this](const std::error_code &ec, size_t /* bytes_transferred */) {
        if (!ec) {
          read_message_ = parser_.parse(read_message_);

          if (current_room_)
            current_room_->onMessageReceived(shared_from_this(), read_message_);
          else
            lobby_->onMessageReceived(shared_from_this(), read_message_);

          do_read_header();
        } else {
          std::cout << "Error read body" << std::endl;
        }
      });
}

void Session::do_write_header() {
  // std::cout << "Session::do_write_header: " << std::endl;
  auto &front_message = write_msgs_.front();
  // std::cout << front_message << std::endl;
  asio::async_write(
      socket_, asio::buffer(&front_message.header, sizeof(server_header)),
      [this](const std::error_code &ec, size_t /* bytes_transferred */) {
        if (!ec) {
          do_write_body();
        } else {
          std::cout << "Error write header" << std::endl;
        }
      });
}

void Session::do_write_body() {
  // std::cout << "Session::do_write_body: " << std::endl;
  auto &front_message = write_msgs_.front();
  // std::cout << "Front Message: " << front_message << std::endl;
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
          // current_room_->leave(shared_from_this());
          std::cout << "Error write body" << std::endl;
        }
      });
}