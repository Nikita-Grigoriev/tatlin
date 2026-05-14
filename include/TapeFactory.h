#pragma once

#include "Tape.h"
#include <memory>

struct TapeFactory {
  [[nodiscard]] virtual std::unique_ptr<Tape>
  create_tmp_tape(std::size_t) const = 0;

  virtual ~TapeFactory() = default;
};
