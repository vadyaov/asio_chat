#pragma once

#include <memory>

#include "../message.hpp"

class Room;
class RoomMgr;

class IParticipant {
public:
  virtual ~IParticipant() {}
  virtual void deliver(const server_message& msg) = 0;

  virtual Room* room() = 0;
  virtual void setRoom(Room* room) = 0;

  virtual RoomMgr* room_mgr() = 0;
};

using participant_ptr = std::shared_ptr<IParticipant>;