#include "FileTape.h"

FileTape::FileTape(const std::string &filepath, const Config &config,
                   std::size_t length)
    : _config(config), _length(length) {
  _file.open(filepath, std::ios::binary | std::ios::in | std::ios::out);

  if (!_file.is_open()) {
    throw TapeError(std::format("Cannot open file {}", filepath));
  }
}

FileTape::~FileTape() noexcept {
  if (_file.is_open()) {
    _file.close();
  }
}

Tape::TapeElement FileTape::read() {
  if (_current_pos >= _length) {
    throw TapeError(
        std::format("Incorrect position to read: {}", _current_pos));
  }

  simulate_delay(_config.read_delay);

  TapeElement value;

  to_position(_current_pos);

  _file.read(reinterpret_cast<char *>(&value), sizeof(TapeElement));

  if (_file.fail()) {
    throw TapeError("Fail reading at position: " +
                    std::to_string(_current_pos));
  }

  return value;
}

void FileTape::write(Tape::TapeElement value) {
  if (_current_pos >= _length) {
    throw TapeError(
        std::format("Incorrect position to write: {}", _current_pos));
  }

  simulate_delay(_config.write_delay);

  to_position(_current_pos);

  _file.write(reinterpret_cast<const char *>(&value), sizeof(TapeElement));

  _file.flush();

  if (_file.fail()) {
    throw TapeError("Write failed at position: " +
                    std::to_string(_current_pos));
  }
}

void FileTape::shift_forward() {
  simulate_delay(_config.shift_delay);
  ++_current_pos;
}

void FileTape::shift_backward() {
  simulate_delay(_config.shift_delay);
  --_current_pos;
}

void FileTape::to_start() {
  simulate_delay(_config.rewind_delay);
  _current_pos = 0;
}

void FileTape::to_end() {
  simulate_delay(_config.rewind_delay);
  _current_pos = _length;
}

void FileTape::simulate_delay(std::chrono::milliseconds delay) {
  std::this_thread::sleep_for(delay);
}

void FileTape::to_position(std::size_t pos) {
  _file.clear();

  _file.seekg(sizeof(Tape::TapeElement) * pos, std::ios::beg);
}

std::size_t FileTape::length() { return _length; }
