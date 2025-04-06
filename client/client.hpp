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

  void do_read_header();
  void do_read_body();
  void do_write_header();
  void do_write_body();

  // dumping answer from server -- REPLACE SOON
  void dump_read();
  
  asio::io_context& io_context_;
  tcp::socket socket_;
  server_message read_message_;
  std::deque<chat_message> write_msgs_;

  MessageParser<server_message, ServerMessageParser<server_message>> parser_;
};
