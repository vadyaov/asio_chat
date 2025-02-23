#include "server.hpp"

#include <iostream>

int main() {
  try {
    asio::io_context io_context;

    tcp::endpoint endpoint(tcp::v4(), 5555);
    chat_server server(io_context, endpoint);

    io_context.run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}