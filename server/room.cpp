#include "room.hpp"

//---------------------------------------ROOM-----------------------------------------//

void ChatRoom::join(participant_ptr participant) {
  LOG_DEBUG("ChatRoom::join:");
  participants_.insert(participant);
  for (auto msg : recent_msgs_) {
    participant->deliver(msg);
  }
}

void ChatRoom::leave(participant_ptr participant) {
  LOG_DEBUG("chat::room::leave");
  participants_.erase(participant);
}

void ChatRoom::onMessageReceived(participant_ptr sender, chat_message& msg) {
  LOG_DEBUG("chat::room::onMessageReceived");
  server_message message;
  switch (msg.header.id) {
    case ChatMessageType::TEXT: {
      message.header.id = ServerResponceType::ROOM_MESSAGE;
      message.header.size = msg.header.size;
      message.body = msg.body;
      break;
    }
    case ChatMessageType::LOGIN:
    case ChatMessageType::LOGOUT:
    case ChatMessageType::CREATE:
    case ChatMessageType::DELETE:
    case ChatMessageType::JOIN:
    case ChatMessageType::LIST: {
      message.header.id = ServerResponceType::INVALID_CONTEXT;
      message.AppendString("room");
      break;
    }
    case ChatMessageType::ROOM: {
      // TODO
      // mb ChatRoom should has a name field?
      // but i dont like duplicate name in map key inside lobby and here
      break;
    }
    case ChatMessageType::QUIT: {
      message.header.id = ServerResponceType::OK;
      message.AppendString(/* room_id + */ "exited.");
      sender->toLobby();
      break;
    }
    default: {
      message.header.id = ServerResponceType::UNKNOWN_REQUEST;
    }
  }


  if (message.header.id != ServerResponceType::ROOM_MESSAGE) {
    sender->deliver(message);
  } else {
    recent_msgs_.push_back(message);
    while (recent_msgs_.size() > max_recent_msgs) {
      recent_msgs_.pop_front();
    }

    for (auto &participant : participants_) {
      participant->deliver(message);
    }
  }
}

//------------------------------------------------------------------------------------//


//--------------------------------------LOBBY-----------------------------------------//

void Lobby::join(participant_ptr participant) {
  LOG_DEBUG("Lobby::join");
  participants_.insert(participant);
}

void Lobby::leave(participant_ptr participant) {
  LOG_DEBUG("Lobby::leave");
  participants_.erase(participant);
}

void Lobby::onMessageReceived(participant_ptr sender, chat_message& msg) {
  LOG_DEBUG("Lobby::onMessageReceived");
  server_message answer;
  switch (msg.header.id) {
    case ChatMessageType::TEXT: {
      answer.header.id = ServerResponceType::INVALID_CONTEXT;
      break;
    }
    case ChatMessageType::LOGIN: {
      answer.AppendString("NOT IMPLEMENTED YET");
      break;
    }
    case ChatMessageType::LOGOUT: {
      answer.AppendString("NOT IMPLEMENTED YET");
      break;
    }
    case ChatMessageType::CREATE: {
      std::string room_id;
      msg.ExtractString(room_id);
      if (room_id.empty()) {
        answer.header.id = ServerResponceType::INCORRECT_BODY;
        answer.AppendString(room_id);
        break;
      }

      if (rooms_.find(room_id) != rooms_.cend()) {
        answer.header.id = ServerResponceType::ALREADY_EXISTS;
        answer.AppendString(room_id);
        break;
      }

      rooms_.emplace(room_id, std::make_unique<ChatRoom>(sender));
      answer.header.id = ServerResponceType::OK;
      answer.AppendString(room_id + ": created");
      break;
    }
    case ChatMessageType::DELETE: {
      std::string room_id;
      msg.ExtractString(room_id);
      if (room_id.empty()) {
        answer.header.id = ServerResponceType::INCORRECT_BODY;
        answer.AppendString(room_id);
        break;
      }

      const auto it = rooms_.find(room_id);
      if (it == rooms_.cend()) {
        answer.header.id = ServerResponceType::NOT_FOUND;
        answer.AppendString(room_id);
        break;
      }

      if (!it->second->isOwner(sender)) {
        answer.header.id = ServerResponceType::FORBIDDEN;
        answer.AppendString("You don't have roots to delete this room.");
      }

      // TODO:
      // participants_in_room must be moved to lobby somehow when deleting room
      // if they are not in any other room
      rooms_.erase(it);
      answer.header.id = ServerResponceType::OK;
      answer.AppendString(room_id + ": deleted");

      break;
    }
    case ChatMessageType::JOIN: {
      std::string room_id;
      msg.ExtractString(room_id);
      if (room_id.empty()) {
        answer.header.id = ServerResponceType::INCORRECT_BODY;
        answer.AppendString(room_id);
        break;
      }

      const auto it = rooms_.find(room_id);
      if (it == rooms_.cend()) {
        answer.header.id = ServerResponceType::NOT_FOUND;
        answer.AppendString(room_id);
        break;
      }

      sender->toRoom(it->second.get());
      answer.header.id = ServerResponceType::OK;
      answer.AppendString(room_id + ": joined");
      
      break;
    }
    case ChatMessageType::LIST: {
      answer.header.id = ServerResponceType::OK;
      if (rooms_.empty()) {
        answer.AppendString("[empty lobby]");
      } else {
        std::string room_list;
        for (const auto& [name, room] : rooms_) {
          room_list += name;
          room_list += ' ';
        }
        answer.AppendString(room_list);
      }
      break;
    }
    case ChatMessageType::ROOM: {
      answer.header.id = ServerResponceType::OK;
      answer.AppendString("lobby");
      break;
    }
    case ChatMessageType::QUIT: {
      // TODO: wnen autorization will be done
      // user can leave (disconnect from lobby)
      // does it mean that user also should disconnect from server?
      answer.AppendString("You can not leave lobby now.");
      break;
    }
    case ChatMessageType::UNKNOWN: {
      answer.header.id = ServerResponceType::UNKNOWN_REQUEST;
    }
  }

  auto participant = participants_.find(sender);
  if (participant != participants_.cend()) {
    (*participant)->deliver(answer);
  }
}

//------------------------------------------------------------------------------------//
