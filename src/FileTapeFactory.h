#pragma once

#include <filesystem>
#include <format>

#include "Config.h"
#include "FileTape.h"
#include "Tape.h"
#include "TapeFactory.h"

struct FileTapeFactory : TapeFactory {
public:
  explicit FileTapeFactory(Config const &config);

  [[nodiscard]] std::unique_ptr<Tape>
  create_tmp_tape(std::size_t length) const override;

private:
  Config _config;
};
