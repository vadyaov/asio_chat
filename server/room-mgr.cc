#include "room-mgr.h"
#include "room.hpp"

#include <algorithm>
#include <mutex>

RoomMgr::RoomMgr() : lobby_(new Lobby) {}

ServerResponceType RoomMgr::createRoom(const std::string& room_id, participant_ptr creator) {
  // std::cout << "RoomMgr::CreateRoom: " << room_id << std::endl;
  if (room_id.empty()) {
    return ServerResponceType::INCORRECT_BODY;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  if (rooms_.find(room_id) != rooms_.cend()) {
    return ServerResponceType::ALREADY_EXISTS;
  }

  rooms_.emplace(room_id, std::make_unique<ChatRoom>(room_id, creator));
  return ServerResponceType::OK;
}

ServerResponceType RoomMgr::deleteRoom(const std::string& room_id, participant_ptr deleter) {
  // std::cout << "Lobby::DeleteRoom: " << room_id << std::endl;
  if (room_id.empty()) {
    return ServerResponceType::INCORRECT_BODY;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  const auto it = rooms_.find(room_id);
  if (it == rooms_.cend()) {
    return ServerResponceType::NOT_FOUND;
  }

  if (!it->second->isOwner(deleter)) {
    return ServerResponceType::FORBIDDEN;
  }

  for (auto participant : rooms_[room_id]->getParticipants()) {
    participant->setRoom(lobby_.get());
  }
  rooms_.erase(it);
  return ServerResponceType::OK;
}

ServerResponceType RoomMgr::moveParticipantToRoom(participant_ptr participant, const std::string& room_id) {
  // std::cout << "RoomMgr::moveParticipantToRoom: " << room_id << std::endl;
  if (room_id.empty()) {
    return ServerResponceType::INCORRECT_BODY;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  const auto it = rooms_.find(room_id);
  if (it == rooms_.cend()) {
    return ServerResponceType::NOT_FOUND;
  }
  participant->setRoom(it->second.get());
  return ServerResponceType::OK;
}

std::vector<std::string> RoomMgr::listRooms() const {
  // std::cout << "RoomMgr::listRooms" << std::endl;
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<std::string> answer;
  std::for_each(rooms_.begin(), rooms_.end(), [&answer](const auto& item) {
    answer.push_back(item.first);
  });
  return answer;
}

Room* RoomMgr::lobby() { return lobby_.get(); }