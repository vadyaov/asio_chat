#pragma once

#include <cstring>
#include <string>
#include <type_traits>
#include <vector>
#include <ostream>

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
struct header {
  T id;
  uint32_t size;
};

template <typename T>
struct message {
  struct header<T> header {};
  std::vector<std::byte> body;

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
    os << "[" << static_cast<uint32_t>(msg.header.id) << ", " << msg.header.size << "]";
    return os;
  }
};
