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
extern uint32_t max_key_length;

// Thread local timer
extern thread_local Timer<std::ratio<1,1000000>> timer;

typedef struct {
  // # of entries in node
  uint32_t node_size_ = 0;

  // # of mutable bits
  uint32_t mutable_size_ = 0;

  // slot lengths
  uint32_t* slot_lengths_;

  // keys
  KeyFieldType* keys_;

  // visibility bits
  volatile bool *visible_;

  // # of entries currently stored in node
  volatile uint32_t offset_ = 0;

  // mutable bits
  volatile uint32_t* mutable_;
} NodeType;

class BTree {
 public:
  BTree(const configuration& state);
  ~BTree();

  // insert item
  uint32_t InsertOffset(const KeyFieldType* key, uint32_t key_len);

  // insert item
  uint32_t InsertMutable(const KeyFieldType* key, uint32_t key_len);

  // dump tree's contents
  void Dump(void);

 public:

  // storage for current node being worked on
  NodeType node_;

};
