#pragma once

#include "message.hpp"
#include <iostream>
#include <ostream>

template<typename Message, typename Derived>
class MessageParser {
public:
  Message parse(const Message& src) {
    // call to derived class imlementation
    return static_cast<Derived*>(this)->parseImpl(src);
  }
protected:
  Message parseImpl(const Message& src) {
    return Message{}; // default dummy
  }
};

template<typename Message>
class ClientMessageParser : public MessageParser<Message, ClientMessageParser<Message>> {
public:
  Message parseImpl(const Message& src) {
    // std::cout << "ClientMessageParser::parse" << std::endl;

    std::string content;
    if (src >> content; content.empty()) return Message();


    /*
        /login <username> <password>  -- login to server
        /logout                       -- logout from server
        /create <room_id>
        /delete <room_id>
        /join <room_id>               -- leave current room and join to another
        /list                         -- list all rooms
        /room                         -- current room
        /quit                         -- disconnect from server
    */

    if (content[0] != '/') {
      return Message(ChatMessageType::TEXT, src);
    }

    // content starts with '/'
    std::vector<std::string> tokens = Split(content.substr(1), " ");
    Message result(GetTypeFromString(tokens[0]));

    if (tokens.size() > 1) {
      int i = 1;
      while (i < tokens.size() - 1) {
        result << tokens[i] << " ";
        i++;
      }
      result << tokens[i];
    }

    return result;
  }
};