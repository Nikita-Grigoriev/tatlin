#include "TapeSorter.h"
#include <algorithm>
#include <array>
#include <deque>
#include <filesystem>
#include <queue>
#include <vector>

namespace {
struct MergeUnit {
  std::unique_ptr<Tape> tape;
  std::deque<std::size_t> chunk_sizes;
};

struct HeapNode {
  bool operator>(HeapNode const &other) const { return value > other.value; }

  Tape::TapeElement value;
  std::size_t tape_id;
};
} // namespace

static void fill_group(std::unique_ptr<Tape> &input,
                       std::vector<MergeUnit> &group, std::size_t chunk_size) {
  std::size_t current_unit = 0;
  std::vector<Tape::TapeElement> buffer;
  buffer.reserve(chunk_size);

  input->to_start();
  while (!input->end()) {
    for (std::size_t i = 0; i < chunk_size && !input->end(); ++i) {
      buffer.push_back(input->read());
      input->shift_forward();
    }

    if (buffer.empty())
      break;
    std::ranges::sort(buffer);

    for (auto i : buffer) {
      group[current_unit].tape->write(i);
      group[current_unit].tape->shift_forward();
    }

    group[current_unit].chunk_sizes.push_back(buffer.size());
    current_unit = (current_unit + 1) % group.size();
    buffer.clear();
  }

  for (auto &&g : group) {
    g.tape->to_start();
  }
}

static bool merge_to_one(std::vector<MergeUnit> &from, MergeUnit &to) {
  std::priority_queue<HeapNode, std::vector<HeapNode>, std::greater<>> heap;

  for (std::size_t i = 0; i < from.size(); ++i) {
    if (!from[i].chunk_sizes.empty()) {
      heap.push({from[i].tape->read(), i});
      from[i].tape->shift_forward();
      --from[i].chunk_sizes.front();
    }
  }

  if (heap.empty()) {
    return false;
  }

  std::size_t new_chunk_size = 0;
  while (!heap.empty()) {
    HeapNode minNode = heap.top();
    heap.pop();

    to.tape->write(minNode.value);
    to.tape->shift_forward();

    ++new_chunk_size;

    if (from[minNode.tape_id].chunk_sizes.front() != 0) {
      heap.push({from[minNode.tape_id].tape->read(), minNode.tape_id});
      from[minNode.tape_id].tape->shift_forward();
      --from[minNode.tape_id].chunk_sizes.front();
    } else {
      from[minNode.tape_id].chunk_sizes.pop_front();
    }
  }

  to.chunk_sizes.push_back(new_chunk_size);

  return true;
}

static void rewind_to_start(std::vector<MergeUnit> &units) {
  for (auto &&unit : units) {
    unit.tape->to_start();
  }
}

static bool merge_step(std::vector<MergeUnit> &from,
                       std::vector<MergeUnit> &to) {
  rewind_to_start(from);
  rewind_to_start(to);

  bool need_more = false;
  for (std::size_t to_index = 0;; to_index = (to_index + 1) % to.size()) {
    if (!merge_to_one(from, to[to_index])) {
      return need_more;
    }
    need_more = true;
  }
}

void TapeSorter::sort(std::unique_ptr<Tape> &input,
                      std::unique_ptr<Tape> &output,
                      std::size_t memory_limit) const {
  std::size_t chunk_size = memory_limit / sizeof(Tape::TapeElement);
  if (chunk_size == 0)
    chunk_size = 1;

  std::size_t tape_length = input->length();
  if (tape_length == 0) {
    output->to_start();
    return;
  }

  std::size_t tmp_tapes = std::min(static_cast<std::size_t>(100),
                                   chunk_size * sizeof(Tape::TapeElement));
  std::size_t one_group = tmp_tapes / 2;

  std::array<std::vector<MergeUnit>, 2> groups;
  groups[0].reserve(one_group);
  groups[1].reserve(one_group);

  for (std::size_t i = 0; i < one_group; ++i) {
    groups[0].push_back(MergeUnit{_factory->create_tmp_tape(tape_length), {}});
    groups[1].push_back(MergeUnit{_factory->create_tmp_tape(tape_length), {}});
  }

  fill_group(input, groups[0], chunk_size);

  std::size_t src_group = 0;
  while (!groups[src_group][1].chunk_sizes.empty()) {
    std::size_t dst_group = (src_group + 1) % 2;
    merge_step(groups[src_group], groups[dst_group]);
    src_group = dst_group;
  }

  output->to_start();
  groups[src_group][0].tape->to_start();

  for (std::size_t i = 0; i < tape_length; ++i) {
    output->write(groups[src_group][0].tape->read());
    output->shift_forward();
    groups[src_group][0].tape->shift_forward();
  }
}
