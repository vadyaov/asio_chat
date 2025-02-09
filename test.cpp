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
  
  int a = 10;
  bool b = false;
  float c = 1.08;

  struct S {
    int d1;
    char d2;
  } d {7, 'c'};

  std::cout << a << " " << b << " " << c << " " << d.d1 << "," << d.d2 << std::endl;

  // std::cout << "int: " << sizeof(int) << std::endl;
  // std::cout << "bool: " << sizeof(bool) << std::endl;
  // std::cout << "float: " << sizeof(float) << std::endl;
  // std::cout << "char: " << sizeof(char) << std::endl;
  // std::cout << "d: " << sizeof(S) << std::endl;

  message << a << b << c << d;

  std::cout << message << std::endl;

  a = 1;
  b = true;
  c = 12312.1;
  d = {1, 'b'};

  message >> d >> c >> b >> a;



  std::cout << message << std::endl;

  std::cout << a << " " << b << " " << c << " " << d.d1 << "," << d.d2 << std::endl;

  return 0;
}