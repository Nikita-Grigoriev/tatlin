#include "Config.h"
#include "FileTape.h"
#include "FileTapeFactory.h"
#include "TapeError.h"
#include "TapeSorter.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cerr << "Invalid number of arguments." << std::endl;
    std::cerr
        << "Usage: " << argv[0]
        << " <input_file> <output_file> <memory_limit_bytes> [config_path]"
        << std::endl;
    return 1;
  }

  std::string input_path = argv[1];
  std::string output_path = argv[2];

  std::size_t memory_limit = 0;
  try {
    memory_limit = std::stoull(argv[3]);
  } catch (std::exception const &e) {
    std::cerr << "Error: Invalid memory limit format: " << argv[3] << std::endl;
    return 1;
  }

  Config config;

  if (argc >= 5)
    config = load_config(argv[4]);

  try {
    if (!std::filesystem::exists(input_path)) {
      std::cerr << "Error: Input file does not exist: " << input_path
                << std::endl;
      return 1;
    }

    std::size_t input_size_bytes = std::filesystem::file_size(input_path);
    if (input_size_bytes % sizeof(Tape::TapeElement) != 0) {
      std::cerr << "Error: Input file size is not aligned with TapeElement "
                   "size constraints."
                << std::endl;
      return 1;
    }

    std::size_t element_count = input_size_bytes / sizeof(Tape::TapeElement);

    {
      std::ofstream out(output_path, std::ios::binary);
      if (!out.is_open()) {
        std::cerr << "Error: Cannot create or open output file: " << output_path
                  << std::endl;
        return 1;
      }
    }

    std::unique_ptr<Tape> input =
        std::make_unique<FileTape>(input_path, config, element_count);
    std::unique_ptr<Tape> output =
        std::make_unique<FileTape>(output_path, config, element_count);

    TapeSorter sorter(std::make_unique<FileTapeFactory>(config));

    sorter.sort(input, output, memory_limit);

    std::cout << "Success!" << std::endl;
  } catch (const TapeError &e) {
    std::cerr << std::endl << "Error: " << e.what() << std::endl;
    return 2;
  }
}
