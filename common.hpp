#pragma once

#include <cstdint>

#include "message.hpp"

#ifdef DEBUG_BUILD
#define LOG_DEBUG(x) std::cerr << x << std::endl;
#else
#define LOG_DEBUG(x) do {} while(0)
#endif

static const char* port = "5555";

enum class ChatMessageType : uint32_t
{
  TEXT,   // need body (rooms only)
  LOGIN,  // NOT IMPLEMENTED YET
  LOGOUT, // NOT IMPLEMENTED YET
  CREATE, // NOT IMPLEMENTED YET
  DELETE, // NOT IMPLEMENTED YET
  LIST,   // no need body (lobby only)
  ROOM,   // no need body (rooms only)
  QUIT,   // no need body (rooms and lobby)
  UNKNOWN
};

using chat_header = chat::message_header<ChatMessageType>;
using chat_message = chat::message<ChatMessageType>;