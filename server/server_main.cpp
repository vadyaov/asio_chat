#include "server.hpp"

#include <iostream>

int main() {
  try {
    asio::io_context io_context;

    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), 5555);
    Server server(io_context, endpoint);

    io_context.run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}