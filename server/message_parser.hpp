#pragma once

#include "../common.hpp"

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

class ClientMessageParser {
public:
  static void parse(chat_message& msg) {
    std::string content;
    msg.ExtractString(content);

    if (content.empty()) return;

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
      msg.header.id = ChatMessageType::TEXT;
      msg.AppendString(content);
      return;
    }

    // content starts with '/'
    std::vector<std::string> tokens = Split(content.substr(1), " ");
    msg.header.id = GetTypeFromString(tokens[0]);

    std::cout << "Type = " << GetStringFromType(msg.header.id) << std::endl;

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
    if (msg.header.id == ChatMessageType::LOGIN ||
        msg.header.id == ChatMessageType::CREATE ||
        msg.header.id == ChatMessageType::DELETE ||
        msg.header.id == ChatMessageType::JOIN) {
      msg.AppendString(body);
    }
  }
};