#include <iostream>
#include <ctime>
#include <asio.hpp>
#include <memory>

/// Async TCP daytime server

using asio::ip::tcp;

std::string make_daytime_string() {
  using namespace std;
  time_t now = time(0);
  return ctime(&now);
}

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
public:
  using pointer = std::shared_ptr<tcp_connection>;

  static pointer create(asio::io_context& io) {
    return pointer(new tcp_connection(io));
  }

  tcp::socket& socket() { return socket_; }

  void start_write() {
    message_ = make_daytime_string();

    asio::async_write(socket_, asio::buffer(message_), std::bind(&tcp_connection::handle_write, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred));
  }


private:
  tcp_connection(asio::io_context& io) : socket_(io) { }

  void handle_write(const std::error_code& /* error */, size_t /* bytes_transferred */) { }

  tcp::socket socket_;

  // message should be here because it must live all the tume while async operation is working
  std::string message_;
};

class tcp_server {
public:
  tcp_server(asio::io_context& io) : io_context_(io),
    acceptor_(io, tcp::endpoint(tcp::v4(), 13))
  {
    start_accept();
  }

private:
  void start_accept() {
    tcp_connection::pointer new_connection = tcp_connection::create(io_context_);

    acceptor_.async_accept(new_connection->socket(), std::bind(&tcp_server::handle_accept, this, new_connection, asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer connection, const std::error_code& error)
  {
    if (!error)
    {
      connection->start_write();
    }
    start_accept();
  }

  asio::io_context& io_context_;
  tcp::acceptor acceptor_;
};

int main() {

  try
  {
    asio::io_context io_context;
    tcp_server server(io_context);
    io_context.run();
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
