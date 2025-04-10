#include "room.hpp"
#include "command.h"

#include <iostream>

Room::~Room() = default;

Room::Room(const std::string& name) : name_(name) {}

void Room::onMessageReceived(participant_ptr sender, chat_message& msg) {
  command_handler_->process(msg, sender);
}

const std::set<participant_ptr>& Room::getParticipants() const noexcept {
  return participants_;
}

//---------------------------------------CHAT-ROOM-----------------------------------------//

ChatRoom::ChatRoom(const std::string& name, participant_ptr creator) : Room(name), creator_(creator) {
  command_handler_ = std::make_unique<ChatRoomCommandHandler>();
  command_handler_->initHandlers();
}

void ChatRoom::join(participant_ptr participant) {
  participants_.insert(participant);
  for (auto msg : recent_msgs_) {
    participant->deliver(msg);
  }
}

void ChatRoom::leave(participant_ptr participant) {
  participants_.erase(participant);
};

void ChatRoom::deliverAll(const server_message& answer) {
  // std::cout << "Delivering all: " << std::endl;
  recent_msgs_.push_back(answer);
  while (recent_msgs_.size() > max_recent_msgs) {
    recent_msgs_.pop_front();
  }

  for (auto& participant : participants_) {
    participant->deliver(answer);
  }
}

//--------------------------------------LOBBY-----------------------------------------//

Lobby::Lobby() : Room("Lobby") {
  command_handler_ = std::make_unique<LobbyCommandHandler>();
  command_handler_->initHandlers();
}

void Lobby::join(participant_ptr participant) {
  participants_.insert(participant);
}

void Lobby::leave(participant_ptr participant) {
  participants_.erase(participant);
}