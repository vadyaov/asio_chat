#include "room.hpp"

//---------------------------------------ROOM-----------------------------------------//

void ChatRoom::join(participant_ptr participant) {
  participants_.insert(participant);
  for (auto msg : recent_msgs_) {
    participant->deliver(msg);
  }
}

void ChatRoom::leave(participant_ptr participant) {
  participants_.erase(participant);
  participant->toLobby();
};

void ChatRoom::onMessageReceived(participant_ptr sender, chat_message& msg) {
  LOG_DEBUG("chat::room::onMessageReceived");
  command_handler_->process(msg, sender, this);
}

void ChatRoom::deliverAll(server_message&& answer) {
  recent_msgs_.push_back(std::forward<server_message>(answer));
  while (recent_msgs_.size() > max_recent_msgs) {
    recent_msgs_.pop_front();
  }

  for (auto& participant : participants_) {
    participant->deliver(answer);
  }
}

//------------------------------------------------------------------------------------//


//--------------------------------------LOBBY-----------------------------------------//

void Lobby::join(participant_ptr participant) {
  participants_.insert(participant);
}

void Lobby::leave(participant_ptr participant) {
  participants_.erase(participant);
  participant->disconnect();
}

void Lobby::onMessageReceived(participant_ptr sender, chat_message& msg) {
  LOG_DEBUG("Lobby::onMessageReceived");
  command_handler_->process(msg, sender, this);
}

//------------------------------------------------------------------------------------//
