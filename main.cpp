#include <iostream>
#include <asio.hpp>

int main() {
  asio::io_context io_context;

  asio::ip::tcp::socket socket(io_context);

  asio::error_code ec;
  socket.connect(server_endpoint, ec);
}