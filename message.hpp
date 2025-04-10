#pragma once

#include <cstring>
#include <string>
#include <type_traits>
#include <vector>
#include <ostream>

enum class ChatMessageType : uint32_t
{
  TEXT = 0,
  LOGIN,  // NOT IMPLEMENTED YET
  LOGOUT, // NOT IMPLEMENTED YET
  REGISTER,
  CREATE,
  DELETE,
  JOIN,
  LIST,
  ROOM,
  QUIT,
  UNKNOWN
};

enum class ServerResponceType : uint32_t
{
  OK = 0,
  INTERNAL_ERROR,
  UNKNOWN_REQUEST,
  INCORRECT_BODY,
  INVALID_CONTEXT,
  ALREADY_EXISTS,
  NOT_FOUND,
  FORBIDDEN,
  INCORRECT_LOGIN,
  INCORRECT_PASSWORD,
  INVALID_CREDENTIALS,
};


template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
struct header {
  T id;
  uint32_t size;
};

template <typename T>
struct message {
  struct header<T> header {};
  std::vector<std::byte> body;

  message() = default;
  message(T id) : header{id, 0} {}

  template<typename Y>
  message(T id, const message<Y>& other) : message(id) {
    header.size = other.header.size;
    body = other.body;
  }

  template<typename Y>
  message(const message<Y>& other) : message(T(), other) {}

  void clear() {
    body.clear();
    header.size = 0;
  }

  message<T>& operator<<(const std::string& src) {
    auto old_size = header.size;
    body.resize(old_size + src.size());
    std::memcpy(body.data() + old_size, src.data(), src.size());
    header.size = body.size();
    return *this;
  }

  const message<T>& operator>>(std::string& dst) const {
    dst.resize(body.size());
    std::memcpy(dst.data(), body.data(), body.size());
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& os, const message<T>& msg)
  {
    std::string text;
    msg >> text;
    os << "[" << GetStringFromType(msg.header.id) << "] " << text;
    return os;
  }
};

using chat_header = header<ChatMessageType>;
using chat_message = message<ChatMessageType>;

using server_header = header<ServerResponceType>;
using server_message = message<ServerResponceType>;

inline std::vector<std::string> Split(const std::string &input,
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

inline ChatMessageType GetTypeFromString(const std::string &str) {
  ChatMessageType type = ChatMessageType::UNKNOWN;

  if (str == "login") {
    type = ChatMessageType::LOGIN;
  } else if (str == "logout") {
    type = ChatMessageType::LOGOUT;
  } else if (str == "register") {
    type = ChatMessageType::REGISTER;
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

inline std::string GetStringFromType(ChatMessageType type) {
  std::string str = "UNKNOWN";

  if (type == ChatMessageType::LOGIN) {
    str = "LOGIN";
  } else if (type == ChatMessageType::LOGOUT) {
    str = "LOGOUT";
  } else if (type == ChatMessageType::REGISTER) {
    str = "REGISTER";
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

inline std::string GetStringFromType(ServerResponceType type) {
  std::string str = "UNKNOWN";

  if (type == ServerResponceType::OK) {
    str = "OK";
  } else if (type == ServerResponceType::ALREADY_EXISTS) {
    str = "already exist";
  } else if (type == ServerResponceType::FORBIDDEN) {
    str = "forbidden";
  } else if (type == ServerResponceType::INCORRECT_BODY) {
    str = "incorrect body";
  } else if (type == ServerResponceType::INTERNAL_ERROR) {
    str = "internal error";
  } else if (type == ServerResponceType::INVALID_CONTEXT) {
    str = "invalid context";
  } else if (type == ServerResponceType::NOT_FOUND) {
    str = "not found";
  } else if (type == ServerResponceType::UNKNOWN_REQUEST) {
    str = "unknown request";
  }

  return str;
}
