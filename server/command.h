#pragma once

#include "participant.hpp"

#include <memory>
#include <unordered_map>

class Room;
class RoomMgr;

class ICommand {
public:
  virtual ~ICommand() {}

  // first argument can be replaced with just std::string (body)
  virtual void execute(chat_message& message, participant_ptr sender) = 0;
};

class RoomTextCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class LobbyTextCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class NotImplementedCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class InvalidContextCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class GetNameCommand : public ICommand {
public:
  void execute(chat_message& message,  participant_ptr sender) override;
};

class QuitCommand : public ICommand {
public:
  QuitCommand(RoomMgr* room_mgr) : room_mgr_(room_mgr) {}
  void execute(chat_message& message, participant_ptr sender) override;
private:
  RoomMgr* room_mgr_;
};

class UnknownCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class CreateRoomCommand : public ICommand {
public:
  CreateRoomCommand(RoomMgr* room_mgr) : room_mgr_(room_mgr) {}
  void execute(chat_message& message, participant_ptr sender) override;
private:
  RoomMgr* room_mgr_;
};

class DeleteRoomCommand : public ICommand {
public:
  DeleteRoomCommand(RoomMgr* room_mgr) : room_mgr_(room_mgr) {}
  void execute(chat_message& message, participant_ptr sender) override;
private:
  RoomMgr* room_mgr_;
};

class JoinRoomCommand : public ICommand {
public:
  JoinRoomCommand(RoomMgr* room_mgr) : room_mgr_(room_mgr) {}
  void execute(chat_message& message, participant_ptr sender) override;
private:
  RoomMgr* room_mgr_;
};

class ListRoomCommand : public ICommand {
public:
  ListRoomCommand(RoomMgr* room_mgr) : room_mgr_(room_mgr) {}
  void execute(chat_message& message, participant_ptr sender) override;
private:
  RoomMgr* room_mgr_;
};

class ICommandFactory {
public:
  virtual ~ICommandFactory() {}
  ICommandFactory(RoomMgr* room_mgr) : room_mgr_(room_mgr) {}
  virtual std::unique_ptr<ICommand> createCommand(ChatMessageType type) = 0;

protected:
  RoomMgr* room_mgr_;
};

class ChatRoomCommandFactory : public ICommandFactory {
public:
  using ICommandFactory::ICommandFactory;
  std::unique_ptr<ICommand> createCommand(ChatMessageType type) override;
};

class LobbyCommandFactory : public ICommandFactory {
public:
  using ICommandFactory::ICommandFactory;
  std::unique_ptr<ICommand> createCommand(ChatMessageType type) override;
};

// Question is that i want to answer to any type of messages. Not only for chat_message.
// Should i rebuild my message class ?
class ICommandHandler {
public:
  virtual ~ICommandHandler() {}

  void initHandlers();

  void process(chat_message& message, participant_ptr sender);

protected:
  std::unordered_map<ChatMessageType, std::unique_ptr<ICommand>> handlers_;
  std::unique_ptr<ICommandFactory> command_factory_;
};

class ChatRoomCommandHandler : public ICommandHandler {
public:
  ChatRoomCommandHandler(RoomMgr* room_mgr) {
    command_factory_ = std::make_unique<ChatRoomCommandFactory>(room_mgr);
  }
};

class LobbyCommandHandler : public ICommandHandler {
public:
  LobbyCommandHandler(RoomMgr* room_mgr) {
    command_factory_ = std::make_unique<LobbyCommandFactory>(room_mgr);
  }
};