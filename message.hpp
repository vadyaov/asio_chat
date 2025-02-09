#pragma once

#include <cstring>
#include <ostream>
#include <vector>
#include <cstdint>

namespace chat
{
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

    size_t size() const
    {
      return sizeof(message_header<T>) + body.size();
    }

    friend std::ostream& operator<<(std::ostream& os, const message<T>& msg)
    {
      os << "ID: " << (uint32_t)msg.header.id << " Size: " << msg.header.size;
      return os;
    }

    // << operator for writing POD-type (int, float, double, struct plain-old-data)
    template<typename DataType>
    friend message<T>& operator<<(message<T>& msg, const DataType& data)
    {
      static_assert(std::is_standard_layout<DataType>::value, "Data is not a standard layout type.");

      msg.body.resize(msg.header.size + sizeof(DataType));
      std::memcpy(msg.body.data() + msg.header.size, &data, sizeof(DataType));

      msg.header.size = msg.body.size();

      return msg;
    }

    template<typename DataType>
    friend message<T>& operator>>(message<T>& msg, DataType& data)
    {

      static_assert(std::is_standard_layout<DataType>::value, "Data is not a standard layout type.");

      if (msg.body.size() < sizeof(DataType))
      {
        std::memset(&data, 0, sizeof(DataType));
      }

      size_t new_size = msg.body.size() - sizeof(DataType);
      std::memcpy(&data, msg.body.data() + new_size, sizeof(DataType));

      msg.body.resize(new_size);
      msg.header.size = msg.body.size();

      return msg;
    }
  };
} // namespace chat
