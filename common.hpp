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
  TEXT,   // need body (rooms only)
  LOGIN,  // NOT IMPLEMENTED YET
  LOGOUT, // NOT IMPLEMENTED YET
  CREATE, // NOT IMPLEMENTED YET
  DELETE, // NOT IMPLEMENTED YET
  JOIN,   // 
  LIST,   // no need body (lobby only)
  ROOM,   // no need body (rooms only)
  QUIT,   // no need body (rooms and lobby)
  UNKNOWN
};

enum class ServerResponceType : uint32_t
{
  OK,
  ROOM_MESSAGE,
  INTERNAL_ERROR,
  UNKNOWN_REQUEST,
  INCORRECT_BODY,
  INVALID_CONTEXT,
  ALREADY_EXISTS,
  NOT_FOUND,
  FORBIDDEN,
};

using chat_header = message_header<ChatMessageType>;
using chat_message = message<ChatMessageType>;

using server_header = message_header<ServerResponceType>;
using server_message = message<ServerResponceType>;