#pragma once

#include <exception>
#include <string>
#include <utility>

class TapeError : public std::exception {
public:
  explicit TapeError(std::string message) : _message(std::move(message)) {}

  [[nodiscard]] const char *what() const noexcept override {
    return _message.c_str();
  }

protected:
  std::string _message;
};
