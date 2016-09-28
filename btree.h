//===----------------------------------------------------------------------===//
// BTREE HEADER
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

// max number of keys in a node
const uint32_t MAX_KEYS = 1000;
const uint32_t MAX_KEYS_PLUS_ONE = MAX_KEYS + 1;

// min number of keys in a node
const uint32_t MIN_KEYS = MAX_KEYS/2;

// null pointer
const long NULL_PTR = -1L;

const uint32_t NUM_MUTABLE_BITS = 64;

typedef uint32_t KeyFieldType;

typedef uint32_t DataFieldType;

extern uint32_t success_count;

typedef struct {
  // keys
  KeyFieldType keys_[MAX_KEYS];

  // fake pointers to child nodes
  DataFieldType values_[MAX_KEYS_PLUS_ONE];

  // number of keys stored in node
  int offset_ = 0;

  // mutable bits
  int mutable_[NUM_MUTABLE_BITS];
} NodeType;

class BTree {
 public:
  BTree();
  ~BTree();

  // insert item
  bool InsertOffset(const KeyFieldType& item);

  // insert item
  bool InsertMutable(const KeyFieldType& item);

  // dump tree's contents
  void Dump(void);

 public:

  // storage for current node being worked on
  NodeType current_node_;
};
