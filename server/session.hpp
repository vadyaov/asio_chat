#pragma once

#include <asio.hpp>

#include "../message_parser.hpp"
#include "auth-manager.hpp"
#include "participant.hpp"
#include "room-mgr.h"
#include "room.hpp"

class Session : public Participant, public std::enable_shared_from_this<Session> {
public:
  using pointer = std::shared_ptr<Session>;
  
  static pointer create(asio::io_context& io_context, RoomMgr* room_mgr, AuthManager* auth_mgr) {
    return pointer(new Session(io_context, room_mgr, auth_mgr));
  }
  
  void start();
  void deliver(const server_message& msg) override;

  void set_room(Room* room) override {
    // std::cout << "session::set_room: " << room->name() << std::endl;
    if (current_room_) {
      current_room_->leave(shared_from_this());
    }
    current_room_ = room;
    current_room_->join(shared_from_this());
  }

  RoomMgr* room_mgr() { return room_mgr_; }
  AuthManager* auth_mgr() { return auth_mgr_; }
  asio::ip::tcp::socket& socket() { return socket_; }
  
private:
  Session(asio::io_context& io, RoomMgr* room_mgr, AuthManager* auth_mgr);
  
  void do_read_header();
  void do_read_body();
  void do_write_header();
  void do_write_body();

  asio::ip::tcp::socket socket_;

  RoomMgr* room_mgr_;
  AuthManager* auth_mgr_;

  chat_message read_message_;
  std::deque<server_message> write_msgs_;

  MessageParser<chat_message, ClientMessageParser<chat_message>> parser_;
};