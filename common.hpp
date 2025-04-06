#pragma once

#include <cstdint>

#include "message.hpp"

#ifdef DEBUG_BUILD
#define LOG_DEBUG(x) std::cerr << x << std::endl;
#else
#define LOG_DEBUG(x) do {} while(0)
#endif

static const char* port = "5555";

// Message from client to server
enum class ChatMessageType : uint32_t
{
  TEXT = 0,
  LOGIN,  // NOT IMPLEMENTED YET
  LOGOUT, // NOT IMPLEMENTED YET
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
};

using chat_header = header<ChatMessageType>;
using chat_message = message<ChatMessageType>;

using server_header = header<ServerResponceType>;
using server_message = message<ServerResponceType>;