#include <asio.hpp>

#include <cstddef>
#include <deque>

#include "../message_parser.hpp"

using asio::ip::tcp;

class chat_client {
public:
  chat_client(asio::io_context& io, tcp::resolver::results_type& endpoints);

  void write(const chat_message& msg);
  
  void close();
  
private:
  void do_connect(const tcp::resolver::results_type& endpoints);
  void handle_connection(const asio::error_code& ec, tcp::endpoint next);

  void do_read_header();
  void handle_read_header(const std::error_code ec, size_t bytes_transfered);

  void do_read_body();
  void handle_read_body(const std::error_code ec, size_t bytes_transfered);

  void do_write_header();
  void handle_write_header(const std::error_code& ec, size_t bytes_transferred);

  void do_write_body();
  void handle_write_body(const std::error_code& ec, size_t bytes_transferred);

  // dumping answer from server -- REPLACE SOON
  void dump_read();
  
  asio::io_context& io_context_;
  tcp::socket socket_;
  server_message read_message_;
  std::deque<chat_message> write_msgs_;

  MessageParser<server_message, ServerMessageParser<server_message>> parser_;
};
