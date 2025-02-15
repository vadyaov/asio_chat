#include <asio.hpp>
#include <cstddef>
#include <deque>
#include <iostream>
#include <set>

#include "common.hpp"


using asio::ip::tcp;

//----------------------------------------------------------------------

class chat_participant {
public:
  virtual ~chat_participant() {}

  virtual void deliver(const chat_message& msg) = 0;
};

using chat_participant_ptr = std::shared_ptr<chat_participant>;

//----------------------------------------------------------------------

class chat_room {
public:
  void join(chat_participant_ptr participant) {
    LOG_DEBUG("chat_room::join:");
    participants_.insert(participant);
    for (auto msg : recent_msgs_)
    {
      participant->deliver(msg);
    }
  }

  void leave(chat_participant_ptr participant) {
    LOG_DEBUG("chat::room::leave");
    participants_.erase(participant);
  }

  void deliver(const chat_message& msg) {
    LOG_DEBUG("chat::room::deliver");
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs) {
      recent_msgs_.pop_front();
    }

    for (auto& participant : participants_) {
      participant->deliver(msg);
    }
  }

private:
  std::set<chat_participant_ptr> participants_;
  enum { max_recent_msgs = 100 };
  std::deque<chat_message> recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session
  : public chat_participant, public std::enable_shared_from_this<chat_session> {
public:
  using pointer = std::shared_ptr<chat_session>;

  static pointer create(asio::io_context& io_context, chat_room& room)
  {
    return pointer(new chat_session(io_context, room));
  }

  void start()
  {
    LOG_DEBUG("chat_session::start");
    room_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const chat_message& msg) override {
    LOG_DEBUG("chat_session::deliver");
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      do_write_header();
    }
  }

  tcp::socket& socket() { return socket_; }

private:
  chat_session(asio::io_context& io, chat_room& room) : socket_(io), room_(room) { }

  void do_read_header() {
    asio::async_read(socket_,
        asio::buffer(&read_message_.header, sizeof(chat_header)),
        std::bind(&chat_session::handle_read_header, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred));
  }

  void handle_read_header(const std::error_code& ec, size_t /*bytes_transferred*/) {
    LOG_DEBUG("chat_session::handle_read_header:");
    if (!ec /* && isCorrectHeader */) {
      do_read_body();
    } else {
      LOG_DEBUG(ec.message());
      room_.leave(shared_from_this());
    }
  }

  void do_read_body() {
    read_message_.body.resize(read_message_.header.size);
    asio::async_read(socket_, asio::buffer(read_message_.body.data(), read_message_.header.size),
        std::bind(&chat_session::handle_read_body, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred));
  }

  void handle_read_body(const std::error_code& ec, size_t /*bytes_transferred*/) {
    LOG_DEBUG("chat_session::handle_read_body:");
    if (!ec) {
      room_.deliver(read_message_);
      do_read_header();
    } else {
      LOG_DEBUG(ec.message());
      room_.leave(shared_from_this());
    }
  }

  void do_write_header() {
    auto& front_message = write_msgs_.front();
    asio::async_write(socket_, asio::buffer(&front_message.header, sizeof(chat_header)),
        std::bind(&chat_session::handle_write_header, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred));
  }

  void handle_write_header(const std::error_code& ec, size_t /* bytes_transferred */) {
    LOG_DEBUG("chat_session::handle_write_header");
    if (!ec) {
      do_write_body();
    } else {
      LOG_DEBUG(ec.message());
      room_.leave(shared_from_this());
    }
  }

  void do_write_body() {
    auto& front_message = write_msgs_.front();
    asio::async_write(socket_, asio::buffer(front_message.body.data(), front_message.header.size),
        std::bind(&chat_session::handle_write_body, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred));
  }

  void handle_write_body(const std::error_code& ec, size_t /* bytes_transferred */) {
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

  tcp::socket socket_;
  chat_room& room_;
  chat_message read_message_;
  std::deque<chat_message> write_msgs_;
};

class chat_server
{
public:
  chat_server(asio::io_context& io, const tcp::endpoint& endpoint)
    : io_context_(io),
      acceptor_(io, endpoint)
  {
    start_accept();
  }


private:
  void start_accept()
  {
    chat_session::pointer new_session = chat_session::create(io_context_, room_);
    acceptor_.async_accept(new_session->socket(),
      std::bind(&chat_server::handle_accept, this, new_session, asio::placeholders::error));
  }

  void handle_accept(chat_session::pointer session, const std::error_code& e)
  {
    LOG_DEBUG("chat_server::handle_accept");
    if (!e)
    {
      session->start();
    }

    start_accept();
  }

  tcp::acceptor acceptor_;
  asio::io_context& io_context_;
  chat_room room_;
};

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