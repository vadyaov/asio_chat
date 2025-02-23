#pragma once

#include <cstddef>
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
  TEXT,
  LOGIN,
  LOGOUT,
  LIST,
  ROOM,
  QUIT,
  UNKNOWN
};

using chat_header = chat::message_header<ChatMessageType>;
using chat_message = chat::message<ChatMessageType>;