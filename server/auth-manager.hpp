#pragma once

#include "participant.hpp"
#include "session.hpp"

#include <string>
#include <unordered_map>

class Session;

class AuthManager {
public:
  struct Credentials {
    std::string login;
    std::string password;
  };
  virtual ~AuthManager() = default;

  virtual ServerResponceType Register(participant_ptr user, const Credentials& credentials) = 0;
  virtual ServerResponceType Authorize(participant_ptr user, const Credentials& credentials) const = 0;
};

// Need to make it thread safe!
class SimpleAuthManager : public AuthManager {
public:
  virtual ServerResponceType Register(participant_ptr user, const Credentials& credentials) override {
    if (credentials.login.empty() || credentials.password.empty()) {
      return ServerResponceType::INVALID_CREDENTIALS;
    }

    if (users_.count(credentials.login)) {
      return ServerResponceType::ALREADY_EXISTS;
    }

    users_[credentials.login] = credentials.password;
    std::cout << "Registered user: " << credentials.login << " " << credentials.password;
    return ServerResponceType::OK;
  }

  virtual ServerResponceType Authorize(participant_ptr user, const Credentials& credentials) const override {
    if (credentials.login.empty() || credentials.password.empty()) {
      return ServerResponceType::INVALID_CREDENTIALS;
    }

    if (!users_.count(credentials.login)) {
      return ServerResponceType::INCORRECT_LOGIN;
    }

    if (users_.at(credentials.login) != credentials.password) {
      return ServerResponceType::INCORRECT_PASSWORD;
    }

    std::cout << "Authorized user: " << credentials.login << " " << credentials.password;
    return ServerResponceType::OK;
  }

private:
  std::unordered_map<std::string, std::string> users_;
};