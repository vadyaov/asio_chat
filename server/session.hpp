#pragma once

#include <asio.hpp>

#include "../message_parser.hpp"
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
  
  void toRoom(IRoom* new_room) override;
  void toLobby() override;

  void disconnect() override;
  
private:
  Session(asio::io_context& io, IRoom* lobby);
  
  void do_read_header();
  void do_read_body();
  void do_write_header();
  void do_write_body();

  asio::ip::tcp::socket socket_;
  IRoom* current_room_;
  IRoom* const lobby_;
  
  chat_message read_message_;
  std::deque<server_message> write_msgs_;

  MessageParser<chat_message, ClientMessageParser<chat_message>> parser_;
};