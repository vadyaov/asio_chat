#pragma once

#include <cstring>
#include <string>
#include <vector>
#include <cstdint>
#include <ostream>

template <typename T>
struct message_header
{
  T id;            // Type of the request message
  uint32_t size;   // Size of the request body [0; n] (in bytes)
};

template <typename T>
struct message
{
  message_header<T> header {};
  std::vector<uint8_t> body;
  uint32_t offset = 0; // pointing to the current pos to start reading from

  size_t size() const
  {
    return sizeof(message_header<T>) + body.size() + sizeof(uint32_t);
  }

  void AppendString(const std::string& str) {
    if (str.empty()) return;

    uint32_t size = static_cast<uint32_t>(str.size());
    uint32_t old_size = static_cast<uint32_t>(body.size());

    body.resize(old_size + size + sizeof(uint32_t));

    std::memcpy(body.data() + old_size, &size, sizeof(uint32_t));
    std::memcpy(body.data() + old_size + sizeof(uint32_t), str.data(), size);

    header.size = body.size();
  }

  void ExtractString(std::string& dst) {
    if (offset + sizeof(uint32_t) > body.size()) {
      return;
    }

    uint32_t sz;
    std::memcpy(&sz, body.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (offset + sz > body.size()) {
      return;
    }

    dst.resize(sz);
    std::memcpy(dst.data(), body.data() + offset, sz);
    offset += sz;
  }

  friend std::ostream& operator<<(std::ostream& os, const message<T>& msg)
  {
    os << "ID: " << (uint32_t)msg.header.id << "; body size: " << msg.header.size << "; offset: " << msg.offset;
    return os;
  }

  // << operator for writing POD-type (int, float, double, struct plain-old-data)
  // template<typename DataType>
  // friend message<T>& operator<<(message<T>& msg, const DataType& data)
  // {
  //   static_assert(std::is_standard_layout<DataType>::value, "Data is not a standard layout type.");

  //   msg.body.resize(msg.header.size + sizeof(DataType));
  //   std::memcpy(msg.body.data() + msg.header.size, &data, sizeof(DataType));

  //   msg.header.size = msg.body.size();

  //   return msg;
  // }

  // template<typename DataType>
  // friend message<T>& operator>>(message<T>& msg, DataType& data)
  // {

  //   static_assert(std::is_standard_layout<DataType>::value, "Data is not a standard layout type.");

  //   if (msg.body.size() < sizeof(DataType))
  //   {
  //     std::memset(&data, 0, sizeof(DataType));
  //     return msg;
  //   }

  //   size_t new_size = msg.body.size() - sizeof(DataType);
  //   std::memcpy(&data, msg.body.data() + new_size, sizeof(DataType));

  //   msg.body.resize(new_size);
  //   msg.header.size = msg.body.size();

  //   return msg;
  // }

};
