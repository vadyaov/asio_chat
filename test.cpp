#include <iostream>
#include <vector>

// #include "message.hpp"

std::vector<std::string> Split(const std::string& src, const std::string& delimiter) {
  std::vector<std::string> tokens;

  size_t current_pos = 0;
  size_t find_pos = 0;
  std::string token;

  while ((find_pos = src.find(delimiter, current_pos)) != std::string::npos && current_pos < src.size()) {
    token = src.substr(current_pos, find_pos - current_pos);
    tokens.push_back(token);
    current_pos = src.find_first_not_of(delimiter, find_pos);
  }
  tokens.push_back(src.substr(current_pos));

  return tokens;
}

int main() {
  // chat::message<MessageType> message;

  // message.header.id = MessageType::DEAFULT;
  
  // std::string str = "Hello, world!";
  
  // message.AppendString(str);
  // message.AppendString("const std::string &str");

  // std::string s1, s2, s3;
  // message.ExtractString(s1);
  // message.ExtractString(s2);
  // message.ExtractString(s3);

  // std::cout << s1 << " " << s2 << std::endl;



  std::string s = "/list adfaf aasf      HELLO WORLD";
    auto splitted = Split(s, " ");
    for (auto& str : splitted)
    std::cout << str << std::endl;

  return 0;
}