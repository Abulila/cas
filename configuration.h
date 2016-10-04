//===----------------------------------------------------------------------===//
// CONFIGURATION
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

enum SynchModeType {
  SYNCH_MODE_TYPE_INVALID = 0,

  SYNCH_MODE_TYPE_OFFSET = 1,
  SYNCH_MODE_TYPE_MUTABLE = 2,
  SYNCH_MODE_TYPE_HYBRID = 3
};

class configuration {
 public:
  // Synch mode type
  SynchModeType synch_mode_type;

  // # of entries in node
  uint32_t node_size;

  // # of entries in mutable bits
  uint32_t mutable_size;

  // # of threads
  uint32_t thread_count;

  // # of loops
  uint32_t loop_count;

  // # of ops
  uint32_t op_count;

  // key size
  uint32_t max_key_size;

};

extern configuration state;
