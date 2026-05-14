#include "Config.h"

#include <fstream>
#include <iostream>

Config load_config(const std::string &config_path) {
  Config config;

  std::ifstream file(config_path);
  if (!file.is_open()) {
    std::cout << "Warning: Cannot find config file. Using default config with "
                 "zero delays."
              << std::endl;
    return config;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    std::size_t delimiter_pos = line.find('=');
    if (delimiter_pos == std::string::npos)
      continue;

    std::string key = line.substr(0, delimiter_pos);
    std::string value_str = line.substr(delimiter_pos + 1);

    auto trim = [](std::string &s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
              }));
      s.erase(std::find_if(s.rbegin(), s.rend(),
                           [](unsigned char ch) { return !std::isspace(ch); })
                  .base(),
              s.end());
    };
    trim(key);
    trim(value_str);

    try {
      std::uint64_t delay = std::stoull(value_str);
      if (key == "read_delay") {
        config.read_delay = std::chrono::milliseconds(delay);
      } else if (key == "write_delay") {
        config.write_delay = std::chrono::milliseconds(delay);
      } else if (key == "shift_delay") {
        config.shift_delay = std::chrono::milliseconds(delay);
      } else if (key == "rewind_delay") {
        config.rewind_delay = std::chrono::milliseconds(delay);
      }
    } catch (const std::exception &) {
      std::cout << "Warning: Invalid format for key <" << key << ">. Skip it."
                << std::endl;
    }
  }

  return config;
}
