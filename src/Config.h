#pragma once

#include <chrono>
#include <string>

struct Config {
  std::chrono::milliseconds read_delay{0};
  std::chrono::milliseconds write_delay{0};
  std::chrono::milliseconds shift_delay{0};
  std::chrono::milliseconds rewind_delay{0};
};

Config load_config(const std::string &config_path);