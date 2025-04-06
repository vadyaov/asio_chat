#pragma once

#include <memory>
#include <set>
#include <map>
#include <deque>

#include "participant.hpp"
#include "../common.hpp"

class ICommandHandler;

class IRoom {
public:
  virtual ~IRoom();
  IRoom(const std::string& name);

  const std::string& name() const { return name_; }

  virtual void join(participant_ptr participant) = 0;
  virtual void leave(participant_ptr participant) = 0;
  virtual void onMessageReceived(participant_ptr sender, chat_message& msg) = 0;

  const std::set<participant_ptr>& getParticipants() const noexcept {
    return participants_;
  }

protected:
  std::string name_;
  std::set<participant_ptr> participants_;
  std::unique_ptr<ICommandHandler> command_handler_;
};

class ChatRoom : public IRoom {
public:
  explicit ChatRoom(const std::string& name, participant_ptr creator);

  void join(participant_ptr participant) override;
  void leave(participant_ptr participant) override;
  void onMessageReceived(participant_ptr sender, chat_message& msg) override;

  bool isOwner(const participant_ptr& someone) { return someone == creator_; }
  void deliverAll(const server_message& answer);
  
private:
  participant_ptr creator_;
  enum { max_recent_msgs = 100 };
  std::deque<server_message> recent_msgs_;
};

class Lobby : public IRoom {
public:
  Lobby();

  void join(participant_ptr participant) override;
  void leave(participant_ptr participant) override;
  void onMessageReceived(participant_ptr sender, chat_message& msg) override;

  ServerResponceType createRoom(const std::string& room_id, participant_ptr creator);
  ServerResponceType deleteRoom(const std::string& room_id, participant_ptr deleter);
  ServerResponceType moveParticipantToRoom(const std::string room_id, participant_ptr joiner) const;

  std::vector<std::string> listRooms() const;

private:
  std::map<std::string, std::unique_ptr<ChatRoom>> rooms_;
};
