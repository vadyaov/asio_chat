#pragma once

#include "participant.hpp"
#include "session.hpp"

#include <netinet/in.h>
#include <string>
#include <unordered_map>

enum class AuthorizationResult {
  OK,
  INCORRECT_LOGIN,
  INCORRECT_PASSWORD,
  INVALID_CREDENTIALS,
};

enum class RegistrationResult {
  OK,
  ALREADY_EXISTS,
  UNSAFE_PASSWORD,
  INVALID_CREDENTIALS,
};

class Session;

class AuthManager {
public:
  struct Credentials {
    std::string login;
    std::string password;
  };
  virtual ~AuthManager() = default;

  virtual RegistrationResult Register(std::shared_ptr<Session> session, participant_ptr user, const Credentials& credentials) = 0;
  virtual AuthorizationResult Authorize(std::shared_ptr<Session> session, participant_ptr user, const Credentials& credentials) const = 0;
};

// Need to make it thread safe!
class SimpleAuthManager : public AuthManager {
public:
  virtual RegistrationResult Register(std::shared_ptr<Session> session, participant_ptr user, const Credentials& credentials) override {
    if (credentials.login.empty() || credentials.password.empty()) {
      return RegistrationResult::INVALID_CREDENTIALS;
    }

    if (users_.count(credentials.login)) {
      return RegistrationResult::ALREADY_EXISTS;
    }

    users_[credentials.login] = credentials.password;
    return RegistrationResult::OK;
  }

  virtual AuthorizationResult Authorize(std::shared_ptr<Session> session, participant_ptr user, const Credentials& credentials) const override {
    if (credentials.login.empty() || credentials.password.empty()) {
      return AuthorizationResult::INVALID_CREDENTIALS;
    }

    if (!users_.count(credentials.login)) {
      return AuthorizationResult::INCORRECT_LOGIN;
    }

    if (users_.at(credentials.login) != credentials.password) {
      return AuthorizationResult::INCORRECT_PASSWORD;
    }

    return AuthorizationResult::OK;
  }

private:
  std::unordered_map<std::string, std::string> users_;
};