#pragma once

#include <asio.hpp>

#include "participant.hpp"
#include "room.hpp"

class Session : public IParticipant, public std::enable_shared_from_this<Session> {
public:
  using pointer = std::shared_ptr<Session>;
  
  static pointer create(asio::io_context& io_context, IRoom* lobby) {
    return pointer(new Session(io_context, lobby));
  }
  
  void start();
  void deliver(const server_message& msg) override;

  asio::ip::tcp::socket& socket() { return socket_; }
  
  void leaveCurrentRoom() {
    if (current_room_)
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
  Session(asio::io_context& io, IRoom* lobby);
  
  void do_read_header();
  void do_read_body();
  void do_write_header();
  void do_write_body();

  void handle_read_header(const std::error_code& ec, size_t bytes_transferred);
  void handle_read_body(const std::error_code& ec, size_t bytes_transferred);
  void handle_write_header(const std::error_code& ec, size_t bytes_transferred);
  void handle_write_body(const std::error_code& ec, size_t bytes_transferred);
  
  asio::ip::tcp::socket socket_;
  IRoom* current_room_;
  IRoom* const lobby_;
  
  chat_message read_message_;
  std::deque<server_message> write_msgs_;
};