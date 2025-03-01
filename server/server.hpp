#pragma once

#include <asio.hpp>
#include <map>
#include <set>
#include <deque>

#include "../common.hpp"

using asio::ip::tcp;

class IRoom;

class chat_participant {
public:
  virtual ~chat_participant() {}

  virtual void deliver(const chat_message& msg) = 0;
  virtual void toRoom(IRoom* room) = 0;
  virtual void toLobby() = 0;
};


using chat_participant_ptr = std::shared_ptr<chat_participant>;

class IRoom {
public:
  virtual ~IRoom() {}
  virtual void join(chat_participant_ptr participant) = 0;
  virtual void leave(chat_participant_ptr participant) = 0;
  virtual void onMessageReceived(chat_participant_ptr sender, chat_message& msg) = 0;
protected:
  std::set<chat_participant_ptr> participants_;
};

class chat_room : public IRoom {
public:
  chat_room(chat_participant_ptr creator) : creator_(creator) {}

  void join(chat_participant_ptr participant) override;
  void leave(chat_participant_ptr participant) override;

  void onMessageReceived(chat_participant_ptr sender, chat_message& msg) override;

  bool isOwner(const chat_participant_ptr& someone) { return someone == creator_; }

private:
  chat_participant_ptr creator_;
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
  void join(chat_participant_ptr participant) override;

  void leave(chat_participant_ptr participant) override;

  void onMessageReceived(chat_participant_ptr sender, chat_message& msg) override;

private:
  std::map<std::string, std::unique_ptr<chat_room>> rooms_;
};


class chat_session
  : public chat_participant, public std::enable_shared_from_this<chat_session> {
public:
  using pointer = std::shared_ptr<chat_session>;

  // user can not create object in other way than calling create and creating shared_ptr
  // --> shared_from_this() will not crash (in case when no control block for this object exists)
  static pointer create(asio::io_context& io_context, IRoom* lobby);

  void start();
  void deliver(const chat_message& msg) override;
  tcp::socket& socket();

  void leaveCurrentRoom() {
    current_room_->leave(shared_from_this());
  }

  void toRoom(IRoom* new_room) override {
    leaveCurrentRoom();
    current_room_ = new_room;
    current_room_->join(shared_from_this());
  }

  void toLobby() override {
    leaveCurrentRoom();
    current_room_ = lobby_;
    current_room_->join(shared_from_this());
  }

private:
  chat_session(asio::io_context& io, IRoom* lobby);

  void do_read_header();
  void handle_read_header(const std::error_code& ec, size_t bytes_transferred);

  void do_read_body();
  void handle_read_body(const std::error_code& ec, size_t bytes_transferred);

  void do_write_header();
  void handle_write_header(const std::error_code& ec, size_t bytes_transferred);

  void do_write_body();
  void handle_write_body(const std::error_code& ec, size_t bytes_transferred);

  tcp::socket socket_;

  IRoom* current_room_;
  IRoom* const lobby_;   

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