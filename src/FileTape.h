#pragma once

#include "Config.h"
#include "Tape.h"
#include "TapeError.h"

#include <fstream>
#include <string>
#include <thread>

class FileTape : public Tape {
public:
  FileTape(std::string const &filepath, Config const &config,
           std::size_t length);

  ~FileTape() noexcept override;

  TapeElement read() override;

  void write(Tape::TapeElement value) override;

  void shift_forward() override;

  void shift_backward() override;

  void to_start() override;

  void to_end() override;

  bool end() override { return _current_pos >= _length; }

  std::size_t length() override;

private:
  static void simulate_delay(std::chrono::milliseconds delay);

  void to_position(std::size_t pos);

  std::fstream _file;
  Config _config;
  std::size_t _current_pos = 0;
  std::size_t _length;
};
