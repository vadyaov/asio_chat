#pragma once

#include "common.hpp"

#include <algorithm>
#include <iostream>

// ----------------------Utils---------------------- //

static std::vector<std::string> Split(const std::string &input,
                                      const std::string &delimiter,
                                      bool skipEmpty = false) {
  std::vector<std::string> tokens;

  if (delimiter.empty()) {
    tokens.push_back(input);
    return tokens;
  }

  size_t start = 0;
  while (true) {
    size_t pos = input.find(delimiter, start);
    if (pos == std::string::npos) {
      if (!skipEmpty || start < input.size()) {
        tokens.push_back(input.substr(start));
      }
      break;
    }

    if (!skipEmpty || pos > start) {
      tokens.push_back(input.substr(start, pos - start));
    }
    start = pos + delimiter.size();
  }

  return tokens;
}

static ChatMessageType GetTypeFromString(const std::string &str) {
  ChatMessageType type = ChatMessageType::UNKNOWN;

  if (str == "login") {
    type = ChatMessageType::LOGIN;
  } else if (str == "logout") {
    type = ChatMessageType::LOGOUT;
  } else if (str == "create") {
    type = ChatMessageType::CREATE;
  } else if (str == "delete") {
    type = ChatMessageType::DELETE;
  } else if (str == "join") {
    type = ChatMessageType::JOIN;
  } else if (str == "list") {
    type = ChatMessageType::LIST;
  } else if (str == "room") {
    type = ChatMessageType::ROOM;
  } else if (str == "quit") {
    type = ChatMessageType::QUIT;
  }

  return type;
}

static std::string GetStringFromType(ChatMessageType type) {
  std::string str = "UNKNOWN";

  if (type == ChatMessageType::LOGIN) {
    str = "LOGIN";
  } else if (type == ChatMessageType::LOGOUT) {
    str = "LOGOUT";
  } else if (type == ChatMessageType::CREATE) {
    str = "CREATE";
  } else if (type == ChatMessageType::DELETE) {
    str = "DELETE";
  } else if (type == ChatMessageType::JOIN) {
    str = "JOIN";
  } else if (type == ChatMessageType::LIST) {
    str = "LIST";
  } else if (type == ChatMessageType::ROOM) {
    str = "ROOM";
  } else if (type == ChatMessageType::QUIT) {
    str = "QUIT";
  }

  return str;
}

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
    std::cout << "ClientMessageParser::parse" << std::endl;
    Message result;

    std::string content;
    if (src >> content; content.empty()) return result;


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
    std::cout << "Type: ";

    if (content[0] != '/') {
      result.header.id = ChatMessageType::TEXT;
      result.header.size = src.header.size;
      result.body = src.body;
      std::cout << GetStringFromType(result.header.id) << std::endl;
      return result;
    }

    // content starts with '/'
    std::vector<std::string> tokens = Split(content.substr(1), " ");
    result.header.id = GetTypeFromString(tokens[0]);

    std::for_each(tokens.begin(), tokens.end(), [](const std::string& token) {
      std::cout << token << " ";
    });
    std::cout << std::endl;

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

    std::string body = tokens.size() > 1 ? tokens[1] : "";
    if (result.header.id == ChatMessageType::LOGIN ||
        result.header.id == ChatMessageType::CREATE ||
        result.header.id == ChatMessageType::DELETE ||
        result.header.id == ChatMessageType::JOIN) {
      // clear all except first token
      result << body;
    }
    std::cout << GetStringFromType(result.header.id) << std::endl;
    return result;
  }
};

template<typename Message>
class ServerMessageParser : public MessageParser<Message, ServerMessageParser<Message>> {
public:
  Message parseImpl(const Message& src) {
    std::cout << "ServerMessageParser::parse" << std::endl;

    Message result;
    result.header.id = ServerResponceType::OK;
    result.header.size = src.header.size;
    result.body = src.body;

    return result;
  }
};