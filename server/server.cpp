#include "server.hpp"

// ----------------------Room----------------------//

void chat_room::join(chat_participant_ptr participant) {
  LOG_DEBUG("chat_room::join:");
  participants_.insert(participant);
  for (auto msg : recent_msgs_) {
    participant->deliver(msg);
  }
}

void chat_room::leave(chat_participant_ptr participant) {
  LOG_DEBUG("chat::room::leave");
  participants_.erase(participant);
}

void chat_room::onMessageReceived(chat_participant_ptr sender, const chat_message& msg) {
  LOG_DEBUG("chat::room::onMessageReceived");
  recent_msgs_.push_back(msg);
  while (recent_msgs_.size() > max_recent_msgs) {
    recent_msgs_.pop_front();
  }

  for (auto &participant : participants_) {
    participant->deliver(msg);
  }
}

// ----------------------Lobby----------------------//

void Lobby::join(chat_participant_ptr participant) {
  participants_.insert(participant);
}

void Lobby::leave(chat_participant_ptr participant) {
  participants_.erase(participant);
}

void Lobby::onMessageReceived(chat_participant_ptr sender, const chat_message& msg) {
  chat_message answer;
  switch (msg.header.id) {
    case ChatMessageType::TEXT: {
      answer.AppendString("You are in lobby. Join to the existing room or create new one");
      break;
    }
    case ChatMessageType::LOGIN: {
      answer.AppendString("NOT IMPLEMENTED YET");
      break;
    }
    case ChatMessageType::LOGOUT: {
      answer.AppendString("NOT IMPLEMENTED YET");
      break;
    }
    case ChatMessageType::LIST: {
      for (const auto& room_ptr : rooms_) {
        answer.AppendString(room_ptr->name());
      }
      break;
    }
    case ChatMessageType::ROOM: {
      answer.AppendString(name());
      break;
    }
    case ChatMessageType::QUIT: {
      answer.AppendString("Leaving " + name());
      break;
    }
    case ChatMessageType::UNKNOWN: {
      answer.AppendString("Unknown command");
    }
  }

  auto participant = participants_.find(sender);
  if (participant != participants_.cend()) {
    (*participant)->deliver(answer);
  }
}

// ----------------------Session----------------------//

chat_session::pointer chat_session::create(asio::io_context &io_context,
                                           IRoom &room) {
  return pointer(new chat_session(io_context, room));
}

void chat_session::start() {
  LOG_DEBUG("chat_session::start");
  room_.join(shared_from_this());
  do_read_header();
}

void chat_session::deliver(const chat_message &msg) {
  LOG_DEBUG("chat_session::deliver");
  bool write_in_progress = !write_msgs_.empty();
  write_msgs_.push_back(msg);
  if (!write_in_progress) {
    do_write_header();
  }
}

tcp::socket &chat_session::socket() { return socket_; }

chat_session::chat_session(asio::io_context& io, IRoom& room)
    : socket_(io), room_(room) {}

void chat_session::do_read_header() {
  asio::async_read(socket_,
                   asio::buffer(&read_message_.header, sizeof(chat_header)),
                   std::bind(&chat_session::handle_read_header,
                             shared_from_this(), asio::placeholders::error,
                             asio::placeholders::bytes_transferred));
}

void chat_session::handle_read_header(const std::error_code &ec, size_t /*bytes_transferred*/) {
  LOG_DEBUG("chat_session::handle_read_header:");
  if (!ec /* && isCorrectHeader */) {
    do_read_body();
  } else {
    LOG_DEBUG(ec.message());
    room_.leave(shared_from_this());
  }
}

void chat_session::do_read_body() {
  read_message_.body.resize(read_message_.header.size);
  asio::async_read(
      socket_,
      asio::buffer(read_message_.body.data(), read_message_.header.size),
      std::bind(&chat_session::handle_read_body, shared_from_this(),
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
}

void chat_session::handle_read_body(const std::error_code &ec,
                                    size_t /*bytes_transferred*/) {
  LOG_DEBUG("chat_session::handle_read_body:");
  if (!ec) {
    room_.onMessageReceived(shared_from_this(), read_message_);
    do_read_header();
  } else {
    LOG_DEBUG(ec.message());
    room_.leave(shared_from_this());
  }
}

void chat_session::do_write_header() {
  auto &front_message = write_msgs_.front();
  asio::async_write(socket_,
                    asio::buffer(&front_message.header, sizeof(chat_header)),
                    std::bind(&chat_session::handle_write_header,
                              shared_from_this(), asio::placeholders::error,
                              asio::placeholders::bytes_transferred));
}

void chat_session::handle_write_header(const std::error_code &ec,
                                       size_t /* bytes_transferred */) {
  LOG_DEBUG("chat_session::handle_write_header");
  if (!ec) {
    do_write_body();
  } else {
    LOG_DEBUG(ec.message());
    room_.leave(shared_from_this());
  }
}

void chat_session::do_write_body() {
  auto &front_message = write_msgs_.front();
  asio::async_write(
      socket_,
      asio::buffer(front_message.body.data(), front_message.header.size),
      std::bind(&chat_session::handle_write_body, shared_from_this(),
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
}

void chat_session::handle_write_body(const std::error_code &ec,
                                     size_t /* bytes_transferred */) {
  LOG_DEBUG("chat_session::handle_write_body");
  if (!ec) {
    write_msgs_.pop_front();
    if (!write_msgs_.empty()) {
      do_write_header();
    }
  } else {
    LOG_DEBUG(ec.message());
    room_.leave(shared_from_this());
  }
}

// ----------------------Server----------------------//

chat_server::chat_server(asio::io_context &io, const tcp::endpoint &endpoint)
    : io_context_(io), acceptor_(io, endpoint) {
  start_accept();
}

void chat_server::start_accept() {
  chat_session::pointer new_session = chat_session::create(io_context_, lobby_);
  acceptor_.async_accept(new_session->socket(),
                         std::bind(&chat_server::handle_accept, this,
                                   new_session, asio::placeholders::error));
}

void chat_server::handle_accept(chat_session::pointer session,
                                const std::error_code &e) {
  LOG_DEBUG("chat_server::handle_accept");
  if (!e) {
    session->start();
  }

  start_accept();
}
