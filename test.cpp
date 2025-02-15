#include <iostream>

#include "message.hpp"

enum class MessageType : uint32_t {
  DEAFULT,
  JOIN,
  LEAVE,
  LIST,
  QUIT,
};


int main() {
  chat::message<MessageType> message;

  message.header.id = MessageType::DEAFULT;
  
  std::string str = "Hello, world!";
  
  message.AppendString(str);
  message.AppendString("const std::string &str");

  std::string s1, s2, s3;
  message.ExtractString(s1);
  message.ExtractString(s2);
  message.ExtractString(s3);

  std::cout << s1 << " " << s2 << std::endl;

  return 0;
}