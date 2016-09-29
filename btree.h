//===----------------------------------------------------------------------===//
// BTREE HEADER
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

#include "configuration.h"
#include "timer.h"

typedef char KeyFieldType;

extern uint32_t success_count;
extern uint32_t found_count;
extern uint32_t out_of_space_count;
extern uint32_t fail_count;
extern uint32_t retry_count;

// Key generation
extern char* key_data;
extern uint32_t fixed_key_length;
extern uint32_t max_offset_length;

typedef struct {
  // # of entries in node
  uint32_t node_size_ = 0;

  // # of mutable bits
  uint32_t mutable_size_ = 0;

  // keys
  KeyFieldType* keys_;

  // visibility bits
  bool *visible_;

  // # of entries currently stored in node
  uint32_t offset_ = 0;

  // mutable bits
  uint32_t* mutable_;
} NodeType;

class BTree {
 public:
  BTree(const configuration& state);
  ~BTree();

  // insert item
  bool InsertOffset(const KeyFieldType* key, uint32_t key_len);

  // insert item
  bool InsertMutable(const KeyFieldType* key, uint32_t key_len);

  // dump tree's contents
  void Dump(void);

  // get duration
  double GetDuration();

 public:

  // storage for current node being worked on
  NodeType node_;

  Timer<> timer;

};
