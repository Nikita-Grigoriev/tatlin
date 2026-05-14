#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <map>
#include <random>
#include <vector>

#include "Config.h"
#include "FileTape.h"
#include "FileTapeFactory.h"
#include "TapeSorter.h"

class ConfigParserTest : public ::testing::Test {
protected:
  std::string valid_config_path = "test_valid_config.txt";
  std::string invalid_config_path = "test_invalid_config.txt";

  void TearDown() override {
    if (std::filesystem::exists(valid_config_path))
      std::filesystem::remove(valid_config_path);
    if (std::filesystem::exists(invalid_config_path))
      std::filesystem::remove(invalid_config_path);
  }
};

TEST_F(ConfigParserTest, ParseValidConfigWithSpacesAndComments) {
  std::ofstream out(valid_config_path);
  out << "read_delay = 15" << std::endl
      << "  write_delay   = 25 " << std::endl
      << "shift_delay=5" << std::endl
      << std::endl
      << "rewind_delay = 120" << std::endl;
  out.close();

  Config cfg = load_config(valid_config_path);

  EXPECT_EQ(cfg.read_delay, std::chrono::milliseconds(15));
  EXPECT_EQ(cfg.write_delay, std::chrono::milliseconds(25));
  EXPECT_EQ(cfg.shift_delay, std::chrono::milliseconds(5));
  EXPECT_EQ(cfg.rewind_delay, std::chrono::milliseconds(120));
}

TEST_F(ConfigParserTest, HandlesInvalidValuesWithoutCrashing) {
  std::ofstream out(invalid_config_path);
  out << "read_delay = text_value_instead_of_number" << std::endl;
  out.close();

  Config cfg = load_config(invalid_config_path);

  EXPECT_EQ(cfg.read_delay, std::chrono::milliseconds(0));
}

TEST(ConfigParserMissingTest, ReturnsDefaultConfigOnMissingFile) {
  Config cfg = load_config("non_existent_file_path.txt");

  EXPECT_EQ(cfg.read_delay, std::chrono::milliseconds(0));
  EXPECT_EQ(cfg.write_delay, std::chrono::milliseconds(0));
  EXPECT_EQ(cfg.shift_delay, std::chrono::milliseconds(0));
  EXPECT_EQ(cfg.rewind_delay, std::chrono::milliseconds(0));
}

struct TapeSorterDiskTest : public ::testing::Test {
public:
  static std::vector<Tape::TapeElement>
  generate_disk_file(const std::string &path, std::size_t count) {
    std::vector<Tape::TapeElement> data(count);
    std::mt19937 gen(42);
    std::uniform_int_distribution<Tape::TapeElement> dist(0, 1000);

    for (auto &val : data) {
      val = dist(gen);
    }

    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char *>(data.data()),
               count * sizeof(Tape::TapeElement));
    file.close();
    return data;
  }

  static std::vector<Tape::TapeElement>
  read_disk_file(const std::string &path) {
    std::vector<Tape::TapeElement> result;
    std::ifstream file(path, std::ios::binary);
    Tape::TapeElement val;
    while (file.read(reinterpret_cast<char *>(&val), sizeof(val))) {
      result.push_back(val);
    }
    return result;
  }

  void test_sort_and_verify(std::size_t num_elements,
                            std::size_t max_elements_in_ram) {
    std::vector<Tape::TapeElement> input_data =
        generate_disk_file(in_path, num_elements);
    std::ofstream(out_path, std::ios::binary).close();

    std::unique_ptr<Tape> input =
        std::make_unique<FileTape>(in_path, config, num_elements);
    std::unique_ptr<Tape> output =
        std::make_unique<FileTape>(out_path, config, num_elements);

    std::size_t memory_limit = max_elements_in_ram * sizeof(Tape::TapeElement);

    TapeSorter sorter(std::make_unique<FileTapeFactory>(config));
    sorter.sort(input, output, memory_limit);

    std::vector<Tape::TapeElement> output_data = read_disk_file(out_path);

    ASSERT_EQ(output_data.size(), input_data.size())
        << "size of output file != size of input file";

    for (std::size_t i = 1; i < output_data.size(); ++i) {
      EXPECT_GE(output_data[i], output_data[i - 1])
          << "Result does not sorted on index: " << i;
    }

    std::map<Tape::TapeElement, std::size_t> counts;
    for (auto val : input_data)
      ++counts[val];
    for (auto val : output_data)
      --counts[val];

    for (auto const &[element, count] : counts) {
      EXPECT_EQ(count, 0) << "Output data has " << element
                          << ", that count != input count";
    }
  }

  void TearDown() override {
    if (std::filesystem::exists(in_path))
      std::filesystem::remove(in_path);
    if (std::filesystem::exists(out_path))
      std::filesystem::remove(out_path);
    std::filesystem::remove_all("tmp");
  }

  std::string in_path = "test_input.bin";
  std::string out_path = "test_output.bin";
  Config config;
};

TEST_F(TapeSorterDiskTest, MultipleEvenChunks) {
  test_sort_and_verify(100, 10);
}

TEST_F(TapeSorterDiskTest, UnevenChunks) { test_sort_and_verify(103, 10); }

TEST_F(TapeSorterDiskTest, MinimumRamLimit) { test_sort_and_verify(50, 1); }

TEST_F(TapeSorterDiskTest, EntireFileFitsInRam) {
  test_sort_and_verify(500, 1000);
}

TEST_F(TapeSorterDiskTest, ManySmallChunks) { test_sort_and_verify(1000, 5); }

TEST_F(TapeSorterDiskTest, EmptyFile) { test_sort_and_verify(0, 10); }

TEST_F(TapeSorterDiskTest, CheckDelays) {
  std::size_t num_elements = 20;
  std::vector<Tape::TapeElement> input_data =
      generate_disk_file(in_path, num_elements);
  std::ofstream(out_path, std::ios::binary).close();

  Config slow_config;
  slow_config.read_delay = std::chrono::milliseconds(5);
  slow_config.write_delay = std::chrono::milliseconds(5);

  std::unique_ptr<Tape> input =
      std::make_unique<FileTape>(in_path, slow_config, num_elements);
  std::unique_ptr<Tape> output =
      std::make_unique<FileTape>(out_path, slow_config, num_elements);

  auto start_time = std::chrono::steady_clock::now();

  TapeSorter sorter(std::make_unique<FileTapeFactory>(slow_config));
  sorter.sort(input, output, 5 * sizeof(Tape::TapeElement));

  auto end_time = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);

  EXPECT_GT(elapsed.count(), 0);
}
