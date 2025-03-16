#include "room.hpp"
#include "command.h"

#include <algorithm>
#include <iostream>

IRoom::~IRoom() = default;

IRoom::IRoom(const std::string& name) : name_(name) {}

//---------------------------------------ROOM-----------------------------------------//

ChatRoom::ChatRoom(const std::string& name, participant_ptr creator) : IRoom(name), creator_(creator) {
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
  participant->toLobby();
};

void ChatRoom::onMessageReceived(participant_ptr sender, chat_message& msg) {
  LOG_DEBUG("chat::room::onMessageReceived");
  command_handler_->process(msg, sender, this);
}

void ChatRoom::deliverAll(const server_message& answer) {
  std::cout << "Delivering all: " << std::endl;
  recent_msgs_.push_back(answer);
  while (recent_msgs_.size() > max_recent_msgs) {
    recent_msgs_.pop_front();
  }

  for (auto& participant : participants_) {
    participant->deliver(answer);
  }
}

//------------------------------------------------------------------------------------//


//--------------------------------------LOBBY-----------------------------------------//

Lobby::Lobby() : IRoom("Lobby") {
  command_handler_ = std::make_unique<LobbyCommandHandler>();
  command_handler_->initHandlers();
}

void Lobby::join(participant_ptr participant) {
  participants_.insert(participant);
}

void Lobby::leave(participant_ptr participant) {
  participants_.erase(participant);
  participant->disconnect();
}

void Lobby::onMessageReceived(participant_ptr sender, chat_message& msg) {
  LOG_DEBUG("Lobby::onMessageReceived");
  command_handler_->process(msg, sender, this);
}

ServerResponceType Lobby::createRoom(const std::string& room_id, participant_ptr creator) {
  std::cout << "Lobby::CreateRoom: " << room_id << std::endl;
  if (room_id.empty()) {
    return ServerResponceType::INCORRECT_BODY;
  }

  if (rooms_.find(room_id) != rooms_.cend()) {
    return ServerResponceType::ALREADY_EXISTS;
  }

  rooms_.emplace(room_id, std::make_unique<ChatRoom>(room_id, creator));
  return ServerResponceType::OK;
}

ServerResponceType Lobby::deleteRoom(const std::string& room_id, participant_ptr deleter) {
  std::cout << "Lobby::DeleteRoom: " << room_id << std::endl;
  if (room_id.empty()) {
    return ServerResponceType::INCORRECT_BODY;
  }

  const auto it = rooms_.find(room_id);
  if (it == rooms_.cend()) {
    return ServerResponceType::NOT_FOUND;
  }

  if (!it->second->isOwner(deleter)) {
    return ServerResponceType::FORBIDDEN;
  }

  // TODO:
  // participants_in_room must be moved to lobby somehow when deleting room
  // if they are not in any other room
  rooms_.erase(it);
  return ServerResponceType::OK;
}

ServerResponceType Lobby::moveParticipantToRoom(const std::string room_id, participant_ptr joiner) const {
  std::cout << "Lobby::moveParticipantToRoom: " << room_id << std::endl;
    if (room_id.empty()) {
      return ServerResponceType::INCORRECT_BODY;
    }

    const auto it = rooms_.find(room_id);
    if (it == rooms_.cend()) {
      return ServerResponceType::NOT_FOUND;
    }
    joiner->toRoom(it->second.get());
    return ServerResponceType::OK;
}

std::vector<std::string> Lobby::listRooms() const {
  std::cout << "Lobby::listRooms" << std::endl;
  std::vector<std::string> answer;
  std::for_each(rooms_.begin(), rooms_.end(), [&answer](const auto& item) {
    answer.push_back(item.first);
  });
  return answer;
}

//------------------------------------------------------------------------------------//
