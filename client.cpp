#include <cstddef>
#include <deque>
#include <iostream>
#include <asio.hpp>
#include <system_error>
#include <thread>

#include "common.hpp"

using asio::ip::tcp;

class chat_client {
public:
  chat_client(asio::io_context& io, tcp::resolver::results_type& endpoints)
    : io_context_(io), socket_(io) {
    do_connect(endpoints);
  }

  void write(const chat_message& msg) {
    asio::post(io_context_, [this, msg]() {
      bool write_in_progress = !write_msgs_.empty();
      write_msgs_.push_back(msg);
      if (!write_in_progress) {
        do_write_header();
      }
    });
  }
  
  void close() { asio::post(io_context_, [this]() { socket_. close(); }); }
  
private:
  void do_connect(const tcp::resolver::results_type& endpoints) {
    asio::async_connect(socket_, endpoints,
      std::bind(&chat_client::handle_connection, this, asio::placeholders::error, asio::placeholders::endpoint));
  }

  void handle_connection(const asio::error_code& ec, tcp::endpoint /* next */) {
    if (!ec) {
      LOG_DEBUG("Connected to server.");
      do_read_header();
    }
  }

  void do_read_header() {
    asio::async_read(socket_,
        asio::buffer((void*)(&read_message_.header), sizeof(chat_header)),
        std::bind(&chat_client::handle_read_header, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
  }

  void handle_read_header(const std::error_code ec, size_t /* bytes_transfered */) {
    LOG_DEBUG("handle_read_header:");
    if (!ec /* && header is good */) {
      read_message_.offset = 0;
      do_read_body();
    } else {
      LOG_DEBUG(ec.message());
      socket_.close();
    }
  }

  void do_read_body() {
    LOG_DEBUG("do_read_body:");
    read_message_.body.resize(read_message_.header.size);
    asio::async_read(socket_,
        asio::buffer(read_message_.body.data(), read_message_.header.size),
        std::bind(&chat_client::handle_read_body, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
  }

  void handle_read_body(const std::error_code ec, size_t /*bytes_transfered*/) {
    LOG_DEBUG("handle_read_body:");
    if (!ec) {
      LOG_DEBUG(read_message);
      dump_read();
      do_read_header();
    } else {
      socket_.close();
    }
  }

  void do_write_header() {
    auto& front_message = write_msgs_.front();
    asio::async_write(socket_, asio::buffer(&front_message.header, sizeof(chat_header)),
        std::bind(&chat_client::handle_write_header, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
  }

  void handle_write_header(const std::error_code& ec, size_t bytes_transferred) {
    LOG_DEBUG("handle_write_header:");
    if (!ec) {
      do_write_body();
    } else {
      LOG_DEBUG(ec.message());
      socket_.close();
    }
  }

  void do_write_body() {
    auto& front_message = write_msgs_.front();
    asio::async_write(socket_, asio::buffer(front_message.body.data(), front_message.header.size),
        std::bind(&chat_client::handle_write_body, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
  }

  void handle_write_body(const std::error_code& ec, size_t /*bytes_transferred*/) {
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

  void dump_read() {
    switch(read_message_.header.id) {
      case ChatMessageType::TEXT: {
        std::string message;
        read_message_.ExtractString(message);
        std::cout << message << std::endl;
        break;
      }
      case ChatMessageType::LOGIN: {

        break;
      }
      case ChatMessageType::LOGOUT: {

        break;
      }
      case ChatMessageType::LIST: {

        break;
      }
      case ChatMessageType::QUIT: {

        break;
      }
      default: {
        LOG_DEBUG("Unknown message type.");
        break;
      }
    }
  }
  
  asio::io_context& io_context_;
  tcp::socket socket_;
  chat_message read_message_;
  std::deque<chat_message> write_msgs_;
};

int main(int argc, char** argv) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: chat_client <host> <port>" << std::endl;
      return 1;
    }

    asio::io_context io_context;

    tcp::resolver resolver(io_context);

    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], argv[2]);
    chat_client client(io_context, endpoints);

    std::thread t([&io_context]() { io_context.run(); });

    
    for (std::string buffer; std::cin >> buffer; ) {
      chat_message message;
      message.AppendString(buffer);
      client.write(message);
    }

    client.close();
    t.join();


  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

}