#include "client.hpp"

#include <iostream>

// ----------------------Utils---------------------- //

static std::vector<std::string> Split(const std::string &input,
                                      const std::string &delimiter,
                                      bool skipEmpty = false) {
  std::vector<std::string> tokens;

  if (delimiter.empty()) {
    tokens.push_back(input);
    return tokens;
  }

  size_t start = 0;
  while (true) {
    size_t pos = input.find(delimiter, start);
    if (pos == std::string::npos) {
      if (!skipEmpty || start < input.size()) {
        tokens.push_back(input.substr(start));
      }
      break;
    }

    if (!skipEmpty || pos > start) {
      tokens.push_back(input.substr(start, pos - start));
    }
    start = pos + delimiter.size();
  }

  return tokens;
}

static ChatMessageType GetTypeFromString(const std::string &str) {
  ChatMessageType type = ChatMessageType::UNKNOWN;

  if (str == "login") {
    type = ChatMessageType::LOGIN;
  } else if (str == "logout") {
    type = ChatMessageType::LOGOUT;
  } else if (str == "create") {
    type = ChatMessageType::CREATE;
  } else if (str == "delete") {
    type = ChatMessageType::DELETE;
  } else if (str == "join") {
    type = ChatMessageType::JOIN;
  } else if (str == "list") {
    type = ChatMessageType::LIST;
  } else if (str == "room") {
    type = ChatMessageType::ROOM;
  } else if (str == "quit") {
    type = ChatMessageType::QUIT;
  }

  return type;
}

static std::string GetStringFromType(ChatMessageType type) {
  std::string str = "UNKNOWN";

  if (type == ChatMessageType::LOGIN) {
    str = "LOGIN";
  } else if (type == ChatMessageType::LOGOUT) {
    str = "LOGOUT";
  } else if (type == ChatMessageType::CREATE) {
    str = "CREATE";
  } else if (type == ChatMessageType::DELETE) {
    str = "DELETE";
  } else if (type == ChatMessageType::JOIN) {
    str = "JOIN";
  } else if (type == ChatMessageType::LIST) {
    str = "LIST";
  } else if (type == ChatMessageType::ROOM) {
    str = "ROOM";
  } else if (type == ChatMessageType::QUIT) {
    str = "QUIT";
  }

  return str;
}


// ----------------------Client---------------------- //

chat_client::chat_client(asio::io_context &io,
                         tcp::resolver::results_type &endpoints)
    : io_context_(io), socket_(io) {
  do_connect(endpoints);
}

chat_message chat_client::CreateMessage(const std::string &buffer) {
  if (buffer.empty())
    return {};

  chat_message message;

  /*
      /login <username> <password>  -- login to server
      /logout                       -- logout from server
      /create <room_id>
      /delete <room_id>
      /join <room_id>               -- leave current room and join to another
      /list                         -- list all rooms
      /room                         -- current room
      /quit                         -- disconnect from server
  */

  if (buffer[0] != '/') {
    message.header.id = ChatMessageType::TEXT;
    message.AppendString(buffer);
  } else {
    std::vector<std::string> tokens = Split(buffer.substr(1), " ");
    message.header.id = GetTypeFromString(tokens[0]);

    /*
      Here is such logic when it doesnt matter what comes after the second
      token. For example: /login <room_id> <some_message> command does not
      take <some_message> (if it exist) to the message body only <room_id> is
      matter

      Maybe change this logic in future: example
      /login <room_id> <first_message to send> -- means login with some "hello
      message" /logout <room_id> <goodbye_message>      -- same /list <limit>
      -- list rooms with limit
    */

    std::string body = tokens.size() > 1 ? tokens[1] : "";
    switch (message.header.id) {
    case ChatMessageType::LOGIN:
    case ChatMessageType::LOGOUT: {
      break;
    }
    case ChatMessageType::CREATE:
    case ChatMessageType::DELETE:
    case ChatMessageType::JOIN: {
      message.AppendString(body);
      break;
    }
    case ChatMessageType::ROOM:
    case ChatMessageType::LIST:
    case ChatMessageType::QUIT: {

      break;
    }
    default: {
      break;
    }
    }
  }

  return message;
}

void chat_client::write(const chat_message &msg) {
  asio::post(io_context_, [this, msg]() {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      do_write_header();
    }
  });
}

void chat_client::close() {
  asio::post(io_context_, [this]() { socket_.close(); });
}

void chat_client::do_connect(const tcp::resolver::results_type &endpoints) {
  asio::async_connect(socket_, endpoints,
                      std::bind(&chat_client::handle_connection, this,
                                asio::placeholders::error,
                                asio::placeholders::endpoint));
}

void chat_client::handle_connection(const asio::error_code &ec,
                                    tcp::endpoint /* next */) {
  if (!ec) {
    LOG_DEBUG("Connected to server.");
    do_read_header();
  }
}

void chat_client::do_read_header() {
  asio::async_read(
      socket_,
      asio::buffer((void *)(&read_message_.header), sizeof(chat_header)),
      std::bind(&chat_client::handle_read_header, this,
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
}

void chat_client::handle_read_header(const std::error_code ec,
                                     size_t /* bytes_transfered */) {
  LOG_DEBUG("handle_read_header:");
  if (!ec /* && header is good */) {
    read_message_.offset = 0;
    do_read_body();
  } else {
    LOG_DEBUG(ec.message());
    socket_.close();
  }
}

void chat_client::do_read_body() {
  LOG_DEBUG("do_read_body:");
  read_message_.body.resize(read_message_.header.size);
  asio::async_read(
      socket_,
      asio::buffer(read_message_.body.data(), read_message_.header.size),
      std::bind(&chat_client::handle_read_body, this, asio::placeholders::error,
                asio::placeholders::bytes_transferred));
}

void chat_client::handle_read_body(const std::error_code ec,
                                   size_t /*bytes_transfered*/) {
  LOG_DEBUG("handle_read_body:");
  if (!ec) {
    LOG_DEBUG(read_message_);
    dump_read();
    do_read_header();
  } else {
    socket_.close();
  }
}

void chat_client::do_write_header() {
  auto &front_message = write_msgs_.front();
  asio::async_write(socket_,
                    asio::buffer(&front_message.header, sizeof(chat_header)),
                    std::bind(&chat_client::handle_write_header, this,
                              asio::placeholders::error,
                              asio::placeholders::bytes_transferred));
}

void chat_client::handle_write_header(const std::error_code &ec,
                                      size_t bytes_transferred) {
  LOG_DEBUG("handle_write_header:");
  if (!ec) {
    do_write_body();
  } else {
    LOG_DEBUG(ec.message());
    socket_.close();
  }
}

void chat_client::do_write_body() {
  auto &front_message = write_msgs_.front();
  asio::async_write(
      socket_,
      asio::buffer(front_message.body.data(), front_message.header.size),
      std::bind(&chat_client::handle_write_body, this,
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
}

void chat_client::handle_write_body(const std::error_code &ec,
                                    size_t /*bytes_transferred*/) {
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

// dumping answer from server
void chat_client::dump_read() {
  std::string message;
  read_message_.ExtractString(message);
  std::cout << message << std::endl;
}