#pragma once

#include "../common.hpp"
#include "participant.hpp"
#include <memory>
#include <unordered_map>

#include "room.hpp"

class ICommand {
public:
  virtual ~ICommand() {}

  // first argument can be replaced with just std::string (body)
  virtual void execute(chat_message& message, participant_ptr sender, IRoom* context) = 0;
};

class RoomTextCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* context) override {
    server_message answer;
    answer.header.id = ServerResponceType::OK;
    answer.header.size = message.header.size;
    answer.body = message.body;

    if (ChatRoom* chat_room = dynamic_cast<ChatRoom*>(context); chat_room) {
      chat_room->deliverAll(std::move(answer));
    }
  }
};

class LobbyTextCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* /*context*/) override {
    server_message answer;
    answer.header.id = ServerResponceType::OK;
    answer.header.size = message.header.size;
    answer.body = message.body;

    sender->deliver(answer);
  }
};

class NotImplementedCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* /*context*/) override {
    server_message answer;
    answer.header.id = ServerResponceType::UNKNOWN_REQUEST;
    answer.AppendString("Not implemented yet.");
    sender->deliver(answer);
  }
};

class InvalidContextCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* /*context*/) override {
    server_message answer;
    answer.header.id = ServerResponceType::INVALID_CONTEXT;
    sender->deliver(answer);
  }
};

class GetNameCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* context) override {
    server_message answer;
    answer.header.id = ServerResponceType::OK;
    answer.AppendString(context->name());
    sender->deliver(answer);
  }
};

class QuitCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* context) override {
    // server_message answer;
    // answer.header.id = ServerResponceType::OK;
    context->leave(sender);
  }
};

class CreateRoomCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* context) override {
    server_message answer;
    std::string room_id;
    if (Lobby* lobby = dynamic_cast<Lobby*>(context); lobby) {
      answer.header.id = lobby->createRoom(room_id, sender);
      if (answer.header.id == ServerResponceType::OK)
        answer.AppendString(room_id + " created.");
    } else {
      answer.header.id = ServerResponceType::INTERNAL_ERROR;
      answer.AppendString("dynamic_cast error");
    }
    sender->deliver(answer);
  }
};

class DeleteRoomCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* context) override {
    server_message answer;
    std::string room_id;
    if (Lobby* lobby = dynamic_cast<Lobby*>(context); lobby) {
      answer.header.id = lobby->deleteRoom(room_id, sender);
      if (answer.header.id == ServerResponceType::OK)
        answer.AppendString(room_id + " created.");
    } else {
      answer.header.id = ServerResponceType::INTERNAL_ERROR;
      answer.AppendString("dynamic_cast error");
    }
    sender->deliver(answer);
  }
};

class JoinRoomCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* context) override {
    server_message answer;
    if (Lobby* lobby = dynamic_cast<Lobby*>(context); lobby) {
      std::string room_id;
      message.ExtractString(room_id);
      answer.header.id = lobby->moveParticipantToRoom(room_id, sender);
      if (answer.header.id == ServerResponceType::OK)
        answer.AppendString("joined room " + room_id);
    } else {
      answer.header.id = ServerResponceType::INTERNAL_ERROR;
      answer.AppendString("dynamic_cast error");
    }
    sender->deliver(answer);
  }
};

class ListRoomCommand : public ICommand {
public:
  void execute(chat_message& message, participant_ptr sender, IRoom* context) override {
    server_message answer;
    if (Lobby* lobby = dynamic_cast<Lobby*>(context); lobby) { 
      answer.header.id = ServerResponceType::OK;
      for (auto& room : lobby->listRooms())
        answer.AppendString(room);
    } else {
      answer.header.id = ServerResponceType::INTERNAL_ERROR;
      answer.AppendString("dynamic_cast error");
    }
    sender->deliver(answer);
  }
};

class ICommandFactory {
public:
  virtual ~ICommandFactory() {}
  virtual std::unique_ptr<ICommand> createCommand(ChatMessageType type) = 0;
};

class ChatRoomCommandFactory : public ICommandFactory {
public:
  std::unique_ptr<ICommand> createCommand(ChatMessageType type) override {
    std::unique_ptr<ICommand> command;
    if (type == ChatMessageType::TEXT) {
      command = std::make_unique<RoomTextCommand>();
    } else if (type == ChatMessageType::LOGIN) {
      command = std::make_unique<NotImplementedCommand>();
    } else if (type == ChatMessageType::LOGOUT) {
      command = std::make_unique<NotImplementedCommand>();
    } else if (type == ChatMessageType::CREATE) {
      command = std::make_unique<InvalidContextCommand>();
    } else if (type == ChatMessageType::DELETE) {
      command = std::make_unique<InvalidContextCommand>();
    } else if (type == ChatMessageType::JOIN) {
      command = std::make_unique<InvalidContextCommand>();
    } else if (type == ChatMessageType::LIST) {
      command = std::make_unique<InvalidContextCommand>();
    } else if (type == ChatMessageType::ROOM) {
      command = std::make_unique<GetNameCommand>();
    } else if (type == ChatMessageType::QUIT) {
      command = std::make_unique<QuitCommand>();
    }
    
    return command;
  }
};

class LobbyCommandFactory : public ICommandFactory {
public:
  std::unique_ptr<ICommand> createCommand(ChatMessageType type) override {
    std::unique_ptr<ICommand> command;
    if (type == ChatMessageType::TEXT) {
      command = std::make_unique<LobbyTextCommand>();
    } else if (type == ChatMessageType::LOGIN) {
      command = std::make_unique<NotImplementedCommand>();
    } else if (type == ChatMessageType::LOGOUT) {
      command = std::make_unique<NotImplementedCommand>();
    } else if (type == ChatMessageType::CREATE) {
      command = std::make_unique<CreateRoomCommand>();
    } else if (type == ChatMessageType::DELETE) {
      command = std::make_unique<DeleteRoomCommand>();
    } else if (type == ChatMessageType::JOIN) {
      command = std::make_unique<JoinRoomCommand>();
    } else if (type == ChatMessageType::LIST) {
      command = std::make_unique<ListRoomCommand>();
    } else if (type == ChatMessageType::ROOM) {
      command = std::make_unique<GetNameCommand>();
    } else if (type == ChatMessageType::QUIT) {
      command = std::make_unique<QuitCommand>();
    }
    
    return command;
  }
};

// Question is that i want to answer to any type of messages. Not only for chat_message.
// Should i rebuild my message class ?
class ICommandHandler {
public:
  virtual ~ICommandHandler() {}

  void initHandlers() {
    handlers_[ChatMessageType::TEXT] = command_factory_->createCommand(ChatMessageType::TEXT);
    handlers_[ChatMessageType::LOGIN] = command_factory_->createCommand(ChatMessageType::LOGIN);
    handlers_[ChatMessageType::LOGOUT] = command_factory_->createCommand(ChatMessageType::LOGOUT);
    handlers_[ChatMessageType::CREATE] = command_factory_->createCommand(ChatMessageType::CREATE);
    handlers_[ChatMessageType::DELETE] = command_factory_->createCommand(ChatMessageType::DELETE);
    handlers_[ChatMessageType::JOIN] = command_factory_->createCommand(ChatMessageType::JOIN);
    handlers_[ChatMessageType::LIST] = command_factory_->createCommand(ChatMessageType::LIST);
    handlers_[ChatMessageType::ROOM] = command_factory_->createCommand(ChatMessageType::ROOM);
    handlers_[ChatMessageType::QUIT] = command_factory_->createCommand(ChatMessageType::QUIT);
  }


  void process(chat_message& message, participant_ptr sender, IRoom* context) {
    handlers_[message.header.id]->execute(message, sender, context);
  }

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