#pragma once

#include "participant.hpp"
#include <memory>
#include <mutex>
#include <unordered_map>

class Room;
class Lobby;

class RoomMgr {
public:
  constexpr static const char * LOBBY_NAME = "!Lobby";
  RoomMgr();

  ServerResponceType createRoom(const std::string& room_id, participant_ptr creator);
  ServerResponceType deleteRoom(const std::string& room_id, participant_ptr deleter);
  ServerResponceType moveParticipantToRoom(participant_ptr participant, const std::string& room_id);

  std::vector<std::string> listRooms() const;

  Lobby* lobby();

private:
  mutable std::mutex mutex_;
  std::unique_ptr<Lobby> lobby_;
  std::unordered_map<std::string, std::unique_ptr<Room>> rooms_;
};