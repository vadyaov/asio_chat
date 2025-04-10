#pragma once

#include <asio.hpp>

#include "../message_parser.hpp"
#include "participant.hpp"
#include "room-mgr.h"
#include "room.hpp"

class Session : public IParticipant, public std::enable_shared_from_this<Session> {
public:
  using pointer = std::shared_ptr<Session>;
  
  static pointer create(asio::io_context& io_context, RoomMgr* room_mgr) {
    return pointer(new Session(io_context, room_mgr));
  }
  
  void start();
  void deliver(const server_message& msg) override;

  Room* room() override { return current_room_; }

  void setRoom(Room* room) override {
    if (current_room_) {
      current_room_->leave(shared_from_this());
    }
    current_room_ = room;
    current_room_->join(shared_from_this());
  }

  RoomMgr* room_mgr() override { return room_mgr_; }

  asio::ip::tcp::socket& socket() { return socket_; }
  
private:
  Session(asio::io_context& io, RoomMgr* room_mgr);
  
  void do_read_header();
  void do_read_body();
  void do_write_header();
  void do_write_body();

  asio::ip::tcp::socket socket_;

  RoomMgr* room_mgr_;
  Room* current_room_ = nullptr;

  chat_message read_message_;
  std::deque<server_message> write_msgs_;

  MessageParser<chat_message, ClientMessageParser<chat_message>> parser_;
};