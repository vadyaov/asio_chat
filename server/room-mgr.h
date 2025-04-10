#pragma once

#include "participant.hpp"
#include <memory>
#include <mutex>
#include <unordered_map>

class IParticipant;
class Room;

class RoomMgr {
public:
  RoomMgr();

  ServerResponceType createRoom(const std::string& room_id, participant_ptr creator);
  ServerResponceType deleteRoom(const std::string& room_id, participant_ptr deleter);
  ServerResponceType moveParticipantToRoom(participant_ptr participant, const std::string& room_id);

  std::vector<std::string> listRooms() const;

  Room* lobby();

private:
  mutable std::mutex mutex_;
  std::unique_ptr<Room> lobby_;
  std::unordered_map<std::string, std::unique_ptr<Room>> rooms_;
};