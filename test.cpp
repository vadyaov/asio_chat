#include "message.hpp"
#include <iostream>

enum MessageType : uint32_t {
  Type1,
  Type2,
  Type3
};

enum class N : int {
  A1, A2
};

using test_message = message<MessageType>;
using wrong_message = message<N>;

int main() {
  test_message text_message;

  text_message << "string1, " << "string 2, " << "string 3";

  std::cout << text_message << std::endl;

  std::string test_string;
  text_message >> test_string;
  std::cout << test_string << std::endl;
  
  return 0;
}