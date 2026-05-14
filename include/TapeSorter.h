#pragma once

#include "TapeFactory.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <queue>
#include <vector>

struct TapeSorter {
  explicit TapeSorter(std::unique_ptr<TapeFactory> &&factory)
      : _factory(std::move(factory)) {}

  void sort(std::unique_ptr<Tape> &input, std::unique_ptr<Tape> &output,
            std::size_t memory_limit) const;

private:
  std::unique_ptr<TapeFactory> _factory;
};
