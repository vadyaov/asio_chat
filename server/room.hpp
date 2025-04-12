#pragma once

#include <memory>
#include <set>
#include <deque>

#include "command.h"
#include "participant.hpp"

class ICommandHandler;

class Room {
public:
  virtual ~Room();
  Room(const std::string& name);

  const std::string& name() const { return name_; }

  virtual void join(participant_ptr participant) = 0;
  virtual void leave(participant_ptr participant) = 0;
  virtual bool isOwner(participant_ptr participant) = 0;

  void onMessageReceived(participant_ptr sender, chat_message& msg);
  const std::set<participant_ptr>& getParticipants() const noexcept;

  void setCommandHandler(std::unique_ptr<ICommandHandler> handler) {
    command_handler_ = std::move(handler);
    command_handler_->initHandlers();
  }

protected:
  std::string name_;
  std::set<participant_ptr> participants_;
  std::unique_ptr<ICommandHandler> command_handler_;
};

// TODO: судя по всему все комнаты должны быть потокобезопасны
// надо подумать, так ли это на самом деле?
class ChatRoom : public Room {
public:
  explicit ChatRoom(const std::string& name, participant_ptr creator);

  void join(participant_ptr participant) override;
  void leave(participant_ptr participant) override;

  bool isOwner(participant_ptr someone) override { return someone.get() == creator_.get(); }
  void deliverAll(const server_message& answer);
  
private:
  participant_ptr creator_;
  enum { max_recent_msgs = 100 };
  std::deque<server_message> recent_msgs_;
};

// Должно ли лобби быть потокобезопасным или нет?
// Если одновременно вызвать join из 2х и более потоков?
// да, думаю все таки тут нужны мьютексы
class Lobby : public Room {
public:
  explicit Lobby(const std::string& name);

  void join(participant_ptr participant) override;
  void leave(participant_ptr participant) override;
  bool isOwner(participant_ptr someone) override { return false; }
};
