#include <ctime>
#include <iostream>
#include <string>

#include <asio.hpp>

/// Sync tcp daytime server

using asio::ip::tcp;

std::string make_daytime_string() {
  using namespace std;

  time_t now = time(0);
  return ctime(&now);
}

int main() {
  try {
    asio::io_context io_context;

    /// A ip::tcp::acceptor object needs to be created to listen for new connections.
    /// It is initialised to listen on TCP port 13, for IP version 4.
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));

    /// This is an iterative server, which means that it will handle one connection at a time.
    for (;;)
    {
      tcp::socket socket(io_context);
      acceptor.accept(socket);

      std::string message = make_daytime_string();

      std::error_code ignored_error;
      asio::write(socket, asio::buffer(message), ignored_error);
    }


  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}