#pragma once

#include "message.hpp"

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

    // std::for_each(tokens.begin(), tokens.end(), [](const std::string& token) {
    //   std::cout << token << " ";
    // });
    // std::cout << std::endl;

    /*
      Here is such logic when it doesnt matter what comes after the second
      token. For example: /login <room_id> <some_message> command does not
      take <some_message> (if it exist) to the message body only <room_id> is
      matter

      Maybe change this logic in future: example
      /login <room_id> <first_message to send> -- means login with some "hello
      message" /logout <room_id> <goodbye_message>      -- same /list <limit>
      -- list rooms with limit
    */

    std::string body = tokens.size() > 1 && result.header.id != ChatMessageType::UNKNOWN ? tokens[1] : tokens[0];
    result << body;
    return result;
  }
};

template<typename Message>
class ServerMessageParser : public MessageParser<Message, ServerMessageParser<Message>> {
public:
  Message parseImpl(const Message& src) {
    // std::cout << "ServerMessageParser::parse" << std::endl;

    std::string dataFromServer;
    src >> dataFromServer;

    Message result;
    result.header.id = src.header.id;
    if (result.header.id != ServerResponceType::OK) {
      result << "Error: " << GetStringFromType(result.header.id) << ": ";
    }
    result << dataFromServer;
    return result;
  }
};