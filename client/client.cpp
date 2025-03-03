#include "client.hpp"

#include <iostream>

chat_client::chat_client(asio::io_context &io,
                         tcp::resolver::results_type &endpoints)
    : io_context_(io), socket_(io) {
  do_connect(endpoints);
}

void chat_client::write(const chat_message &msg) {
  asio::post(io_context_, [this, msg]() {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      do_write_header();
    }
  });
}

void chat_client::close() {
  asio::post(io_context_, [this]() { socket_.close(); });
}

void chat_client::do_connect(const tcp::resolver::results_type &endpoints) {
  asio::async_connect(socket_, endpoints,
                      std::bind(&chat_client::handle_connection, this,
                                asio::placeholders::error,
                                asio::placeholders::endpoint));
}

void chat_client::handle_connection(const asio::error_code &ec,
                                    tcp::endpoint /* next */) {
  if (!ec) {
    LOG_DEBUG("Connected to server.");
    do_read_header();
  }
}

void chat_client::do_read_header() {
  asio::async_read(
      socket_,
      asio::buffer(&read_message_.header, sizeof(server_header)),
      std::bind(&chat_client::handle_read_header, this,
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
}

void chat_client::handle_read_header(const std::error_code ec,
                                     size_t /* bytes_transfered */) {
  LOG_DEBUG("handle_read_header:");
  if (!ec /* && header is good */) {
    read_message_.offset = 0;
    do_read_body();
  } else {
    LOG_DEBUG(ec.message());
    socket_.close();
  }
}

void chat_client::do_read_body() {
  LOG_DEBUG("do_read_body:");
  read_message_.body.resize(read_message_.header.size);
  asio::async_read(
      socket_,
      asio::buffer(read_message_.body.data(), read_message_.header.size),
      std::bind(&chat_client::handle_read_body, this, asio::placeholders::error,
                asio::placeholders::bytes_transferred));
}

void chat_client::handle_read_body(const std::error_code ec,
                                   size_t /*bytes_transfered*/) {
  LOG_DEBUG("handle_read_body:");
  if (!ec) {
    LOG_DEBUG(read_message_);
    dump_read();
    do_read_header();
  } else {
    socket_.close();
  }
}

void chat_client::do_write_header() {
  auto &front_message = write_msgs_.front();
  asio::async_write(socket_,
                    asio::buffer(&front_message.header, sizeof(chat_header)),
                    std::bind(&chat_client::handle_write_header, this,
                              asio::placeholders::error,
                              asio::placeholders::bytes_transferred));
}

void chat_client::handle_write_header(const std::error_code &ec,
                                      size_t bytes_transferred) {
  LOG_DEBUG("handle_write_header:");
  if (!ec) {
    do_write_body();
  } else {
    LOG_DEBUG(ec.message());
    socket_.close();
  }
}

void chat_client::do_write_body() {
  auto &front_message = write_msgs_.front();
  asio::async_write(
      socket_,
      asio::buffer(front_message.body.data(), front_message.header.size),
      std::bind(&chat_client::handle_write_body, this,
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
}

void chat_client::handle_write_body(const std::error_code &ec,
                                    size_t /*bytes_transferred*/) {
  LOG_DEBUG("handle_write_body:");
  if (!ec) {
    write_msgs_.pop_front();
    if (!write_msgs_.empty()) {
      do_write_header();
    }
  } else {
    LOG_DEBUG(ec.message());
    socket_.close();
  }
}

// dumping answer from server
void chat_client::dump_read() {
  std::string message;
  read_message_.ExtractString(message);
  std::cout << message << std::endl;
}