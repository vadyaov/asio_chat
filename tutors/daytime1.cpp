#include <iostream>
#include <vector>
#include <asio.hpp>

/// Sync tcp daytime client

using asio::ip::tcp;

#define BATCH_SIZE 256

int main(int argc, char** argv)
{
  try {
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    asio::io_context io_context;

    // need this to turn server name from agrv to real tcp endpoint
    tcp::resolver resolver(io_context);

    // turns host and service into the list of endpoints
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "daytime");

    tcp::socket socket(io_context);
    asio::connect(socket, endpoints);

    std::cout << "Connected: " << endpoints->host_name() << '.' << endpoints->service_name() << std::endl;

    while (true) {
        std::vector<char> buffer;
        asio::error_code error;

        buffer.resize(BATCH_SIZE, '/');

        size_t len = socket.read_some(asio::buffer(buffer), error);

      if (error == asio::error::eof) {
        std::cout << "Connection closed cleanly by peer\n";
        break; // Connection closed cleanly by peer
      } else if (error) {
        throw std::system_error(error);
      }

      std::cout.write(buffer.data(), len);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}