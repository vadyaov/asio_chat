#include "command.h"
#include "room.hpp"
#include "room-mgr.h"

#include <algorithm>
#include <iostream>

void RoomTextCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing RoomTextCommand:" << std::endl;
  server_message answer(message);
  if (ChatRoom* chat_room = dynamic_cast<ChatRoom*>(sender->room()); chat_room) {
    chat_room->deliverAll(std::move(answer));
  }
}

void LobbyTextCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing LobbyTextCommand:" << std::endl;
  server_message answer(message);
  sender->deliver(answer);
}

void NotImplementedCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing NotImplementedCommand:" << std::endl;
  server_message answer(ServerResponceType::UNKNOWN_REQUEST, message);
  sender->deliver(answer);
}

void InvalidContextCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing InvalidContextCommand:" << std::endl;

  server_message answer(ServerResponceType::INVALID_CONTEXT, message);
  sender->deliver(answer);
}

void GetNameCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing GetNameCommand:" << std::endl;
  server_message answer;
  answer << sender->room()->name();
  sender->deliver(answer);
}

void QuitCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing QuitCommand:" << std::endl;
  server_message answer;
  sender->setRoom(sender->room_mgr()->lobby());
  sender->deliver(answer);
}

void UnknownCommand::execute(chat_message& message, participant_ptr sender) {
  server_message answer(ServerResponceType::UNKNOWN_REQUEST, message);
  sender->deliver(answer);
}

void CreateRoomCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing CreateRoomCommand:" << std::endl;
  server_message answer;
  std::string room_id;
  message >> room_id;
  answer.header.id = sender->room_mgr()->createRoom(room_id, sender);
  if (answer.header.id != ServerResponceType::OK) {
    answer << room_id;
  }
  sender->deliver(answer);
}

void DeleteRoomCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing DeleteRoomCommand:" << std::endl;
  server_message answer;
  std::string room_id;
  message >> room_id;
  answer.header.id = sender->room_mgr()->deleteRoom(room_id, sender);
  if (answer.header.id != ServerResponceType::OK)
    answer << room_id;
  sender->deliver(answer);
}

void JoinRoomCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing JoinRoomCommand:" << std::endl;
  server_message answer;
  std::string room_id;
  message >> room_id;
  answer.header.id = sender->room_mgr()->moveParticipantToRoom(sender, room_id);
  if (answer.header.id != ServerResponceType::OK) {
    answer << room_id;
  }
  sender->deliver(answer);
}

void ListRoomCommand::execute(chat_message& message, participant_ptr sender) {
  // std::cout << "Executing ListRoomCommand:" << std::endl;
  server_message answer;
    std::string all_rooms_string;
    all_rooms_string.push_back('[');
    if (auto rooms = sender->room_mgr()->listRooms(); !rooms.empty()) {
      std::for_each_n(rooms.begin(), rooms.size() - 1,  [&all_rooms_string](const std::string& room) {
        all_rooms_string.append(room + ", ");
      });
      all_rooms_string.append(rooms.back());
    }
    all_rooms_string.push_back(']');
    answer << all_rooms_string;
  sender->deliver(answer);
}

std::unique_ptr<ICommand> ChatRoomCommandFactory::createCommand(ChatMessageType type) {
  // std::cout << "Executing ChatRoomCommandFactory::CreateCommand" << std::endl;
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
  } else if (type == ChatMessageType::UNKNOWN) {
    command = std::make_unique<UnknownCommand>();
  }
  
  return command;
}

std::unique_ptr<ICommand> LobbyCommandFactory::createCommand(ChatMessageType type) {
  // std::cout << "Executing LobbyCommandFactory::CreateCommand" << std::endl;
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
  } else if (type == ChatMessageType::UNKNOWN) {
    command = std::make_unique<UnknownCommand>();
  }
  
  return command;
}

void ICommandHandler::initHandlers() {
  handlers_[ChatMessageType::TEXT] = command_factory_->createCommand(ChatMessageType::TEXT);
  handlers_[ChatMessageType::LOGIN] = command_factory_->createCommand(ChatMessageType::LOGIN);
  handlers_[ChatMessageType::LOGOUT] = command_factory_->createCommand(ChatMessageType::LOGOUT);
  handlers_[ChatMessageType::CREATE] = command_factory_->createCommand(ChatMessageType::CREATE);
  handlers_[ChatMessageType::DELETE] = command_factory_->createCommand(ChatMessageType::DELETE);
  handlers_[ChatMessageType::JOIN] = command_factory_->createCommand(ChatMessageType::JOIN);
  handlers_[ChatMessageType::LIST] = command_factory_->createCommand(ChatMessageType::LIST);
  handlers_[ChatMessageType::ROOM] = command_factory_->createCommand(ChatMessageType::ROOM);
  handlers_[ChatMessageType::QUIT] = command_factory_->createCommand(ChatMessageType::QUIT);
  handlers_[ChatMessageType::UNKNOWN] = command_factory_->createCommand(ChatMessageType::UNKNOWN);
}

void ICommandHandler::process(chat_message& message, participant_ptr sender) {
  if (handlers_.count(message.header.id) == 0) {
    std::cout << "Something bad gonna happen" << std::endl;
  }
  handlers_[message.header.id]->execute(message, sender);
}