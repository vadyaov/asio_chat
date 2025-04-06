#include "client.hpp"

#include <iostream>

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

    
    for (std::string buffer; std::getline(std::cin, buffer); ) {
      chat_message message;
      // message.header.id = ChatMessageType::UNKNOWN;
      message << buffer;
      client.write(message);
    }

    client.close();
    t.join();


  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

}