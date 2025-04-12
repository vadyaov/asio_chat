#pragma once

#include <memory>

#include "../message.hpp"

class Room;

class Participant {
public:
  virtual ~Participant() {}
  virtual void deliver(const server_message& msg) = 0;

  Room* room() { return current_room_; }
  const Room* room() const { return current_room_; }

  virtual void set_room(Room* room) = 0;

protected:
  Room* current_room_ = nullptr;
};

using participant_ptr = std::shared_ptr<Participant>;