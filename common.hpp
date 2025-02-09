#pragma once

#include <cstdint>

static const char* port = "5555";

enum class ChatMessageType : uint32_t
{
  TEXT,
  LOGIN,
  LOGOUT,
  LIST,
  QUIT,
};
