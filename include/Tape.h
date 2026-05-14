#pragma once

#include <cstdint>
#include <optional>

class Tape {
public:
  using TapeElement = std::int32_t;

  virtual ~Tape() = default;

  virtual TapeElement read() = 0;

  virtual void write(TapeElement element) = 0;

  virtual void shift_forward() = 0;

  virtual void shift_backward() = 0;

  virtual void to_start() = 0;

  virtual void to_end() = 0;

  virtual bool end() = 0;

  virtual std::size_t length() = 0;
};
