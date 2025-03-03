#pragma once

#include <set>
#include <map>
#include <deque>

#include "participant.hpp"
#include "../common.hpp"

class IRoom {
public:
  virtual ~IRoom() {}
  virtual void join(participant_ptr participant) = 0;
  virtual void leave(participant_ptr participant) = 0;
  virtual void onMessageReceived(participant_ptr sender, chat_message& msg) = 0;
protected:
  std::set<participant_ptr> participants_;
};

class ChatRoom : public IRoom {
public:
  explicit ChatRoom(participant_ptr creator) : creator_(creator) {}

  void join(participant_ptr participant) override;
  void leave(participant_ptr participant) override;
  void onMessageReceived(participant_ptr sender, chat_message& msg) override;
  bool isOwner(const participant_ptr& someone) { return someone == creator_; }
  
private:
  participant_ptr creator_;
  enum { max_recent_msgs = 100 };
  std::deque<server_message> recent_msgs_;
};

class Lobby : public IRoom {
public:
  void join(participant_ptr participant) override;
  void leave(participant_ptr participant) override;
  void onMessageReceived(participant_ptr sender, chat_message& msg) override;

private:
  std::map<std::string, std::unique_ptr<ChatRoom>> rooms_;
};
