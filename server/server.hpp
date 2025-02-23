#pragma once

#include <asio.hpp>
#include <set>
#include <deque>

#include "../common.hpp"

using asio::ip::tcp;

class chat_participant {
public:
  virtual ~chat_participant() {}

  virtual void deliver(const chat_message& msg) = 0;
};


using chat_participant_ptr = std::shared_ptr<chat_participant>;

class IRoom {
public:
  virtual ~IRoom() {}
  virtual std::string name() const = 0;
  virtual void join(chat_participant_ptr participant) = 0;
  virtual void leave(chat_participant_ptr participant) = 0;
  virtual void onMessageReceived(chat_participant_ptr sender, const chat_message& msg) = 0;
protected:
  std::set<chat_participant_ptr> participants_;
};

class chat_room : public IRoom {
public:
  chat_room(const std::string& name) : name_(name) {}

  std::string name() const override { return name_; }

  void join(chat_participant_ptr participant) override;
  void leave(chat_participant_ptr participant) override;

  void onMessageReceived(chat_participant_ptr sender, const chat_message& msg) override;
private:
  std::string name_;
  enum { max_recent_msgs = 100 };
  std::deque<chat_message> recent_msgs_;
};

/*
  Lobby is not for chatting.
  It is only for client-server talking.
  Being in lobby, you can list rooms, join room, create/delete room if you have rights for this (TODO)
*/
class Lobby : public IRoom {
public:
  std::string name() const override { return "--Lobby--"; }

  void join(chat_participant_ptr participant) override;

  void leave(chat_participant_ptr participant) override;

  void onMessageReceived(chat_participant_ptr sender, const chat_message& msg) override;

private:
  std::vector<std::unique_ptr<chat_room>> rooms_;
};


class chat_session
  : public chat_participant, public std::enable_shared_from_this<chat_session> {
public:
  using pointer = std::shared_ptr<chat_session>;

  static pointer create(asio::io_context& io_context, IRoom& room);

  void start();
  void deliver(const chat_message& msg) override;
  tcp::socket& socket();

private:
  chat_session(asio::io_context& io, IRoom& room);

  void do_read_header();
  void handle_read_header(const std::error_code& ec, size_t bytes_transferred);

  void do_read_body();
  void handle_read_body(const std::error_code& ec, size_t bytes_transferred);

  void do_write_header();
  void handle_write_header(const std::error_code& ec, size_t bytes_transferred);

  void do_write_body();
  void handle_write_body(const std::error_code& ec, size_t bytes_transferred);

  tcp::socket socket_;

  IRoom& room_;
  chat_message read_message_;
  std::deque<chat_message> write_msgs_;
};

class chat_server {
public:
  chat_server(asio::io_context& io, const tcp::endpoint& endpoint);

private:
  void start_accept();
  void handle_accept(chat_session::pointer session, const std::error_code& e);

  tcp::acceptor acceptor_;
  asio::io_context& io_context_;

  Lobby lobby_;
};