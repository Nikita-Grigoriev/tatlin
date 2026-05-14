#include "FileTapeFactory.h"

FileTapeFactory::FileTapeFactory(const Config &config) : _config(config) {
  if (!std::filesystem::exists("tmp")) {
    std::filesystem::create_directories("tmp");
  }
}

std::unique_ptr<Tape>
FileTapeFactory::create_tmp_tape(std::size_t length) const {
  static std::size_t count_tmp_tapes = 0;

  std::string filepath = std::format("tmp/tmp_{}.bin", count_tmp_tapes);

  ++count_tmp_tapes;

  if (!std::filesystem::exists(filepath)) {
    std::ofstream create(filepath, std::ios::binary);
    create.close();
  }

  return std::make_unique<FileTape>(filepath, _config, length);
}
