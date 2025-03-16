#pragma once

#include <memory>
#include "../common.hpp"

class IRoom;

class IParticipant {
public:
  virtual ~IParticipant() {}
  virtual void deliver(const server_message& msg) = 0;
  virtual void toRoom(IRoom* room) = 0;
  virtual void toLobby() = 0;
  virtual void disconnect() = 0;
};

using participant_ptr = std::shared_ptr<IParticipant>;