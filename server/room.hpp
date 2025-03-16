#pragma once

#include <algorithm>
#include <memory>
#include <set>
#include <map>
#include <deque>

#include "command.h"
#include "participant.hpp"
#include "../common.hpp"

class IRoom {
public:
  virtual ~IRoom() {}

  IRoom(const std::string& name) : name_(name) {}

  virtual void join(participant_ptr participant) = 0;

  virtual void leave(participant_ptr participant) = 0;

  const std::string& name() const { return name_; }

  virtual void onMessageReceived(participant_ptr sender, chat_message& msg) = 0;

protected:
  std::string name_;
  std::set<participant_ptr> participants_;
  std::unique_ptr<ICommandHandler> command_handler_;
};

class ChatRoom : public IRoom {
public:
  explicit ChatRoom(const std::string& name, participant_ptr creator) :
    IRoom(name),
    creator_(creator) {
    command_handler_ = std::make_unique<ChatRoomCommandHandler>();
    command_handler_->initHandlers();
  }

  void join(participant_ptr participant) override;
  void leave(participant_ptr participant) override;
  void onMessageReceived(participant_ptr sender, chat_message& msg) override;

  bool isOwner(const participant_ptr& someone) { return someone == creator_; }
  void deliverAll(server_message&& answer);
  
private:
  participant_ptr creator_;
  enum { max_recent_msgs = 100 };
  std::deque<server_message> recent_msgs_;
};

class Lobby : public IRoom {
public:
  Lobby() : IRoom("Lobby") {
    command_handler_ = std::make_unique<LobbyCommandHandler>();
    command_handler_->initHandlers();
  }

  void join(participant_ptr participant) override;
  void leave(participant_ptr participant) override;
  void onMessageReceived(participant_ptr sender, chat_message& msg) override;

  ServerResponceType createRoom(const std::string& room_id, participant_ptr creator) {
    if (room_id.empty()) {
      return ServerResponceType::INCORRECT_BODY;
    }

    if (rooms_.find(room_id) != rooms_.cend()) {
      return ServerResponceType::ALREADY_EXISTS;
    }

    rooms_.emplace(room_id, std::make_unique<ChatRoom>(room_id, creator));
    return ServerResponceType::OK;
  }

  ServerResponceType deleteRoom(const std::string& room_id, participant_ptr deleter) {
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

  ServerResponceType moveParticipantToRoom(const std::string room_id, participant_ptr joiner) const {
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

  std::vector<std::string> listRooms() const {
    std::vector<std::string> answer;
    std::for_each(rooms_.begin(), rooms_.end(), [&answer](const auto& item) {
      answer.push_back(item.first);
    });
    return answer;
  }

private:
  std::map<std::string, std::unique_ptr<ChatRoom>> rooms_;
};
