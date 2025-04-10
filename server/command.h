#pragma once

#include "participant.hpp"

#include <memory>
#include <unordered_map>

class Room;

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
  void execute(chat_message& message, participant_ptr sender) override;
};

class QuitCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class UnknownCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class CreateRoomCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class DeleteRoomCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class JoinRoomCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class ListRoomCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender) override;
};

class ICommandFactory {
public:
  virtual ~ICommandFactory() {}
  virtual std::unique_ptr<ICommand> createCommand(ChatMessageType type) = 0;
};

class ChatRoomCommandFactory : public ICommandFactory {
public:
  std::unique_ptr<ICommand> createCommand(ChatMessageType type) override;
};

class LobbyCommandFactory : public ICommandFactory {
public:
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
  ChatRoomCommandHandler() {
    command_factory_ = std::make_unique<ChatRoomCommandFactory>();
  }
};

class LobbyCommandHandler : public ICommandHandler {
public:
  LobbyCommandHandler() {
    command_factory_ = std::make_unique<LobbyCommandFactory>();
  }
};