//===----------------------------------------------------------------------===//
// BTREE HEADER
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

// max number of keys in a node
const uint32_t MAX_KEYS = 1000000;
const uint32_t MAX_KEYS_PLUS_ONE = MAX_KEYS + 1;

// min number of keys in a node
const uint32_t MIN_KEYS = MAX_KEYS/2;

// null pointer
const long NULL_PTR = -1L;

typedef uint32_t KeyFieldType;

typedef uint32_t DataFieldType;

typedef struct {
  // keys
  KeyFieldType keys_[MAX_KEYS];

  // fake pointers to child nodes
  DataFieldType values_[MAX_KEYS_PLUS_ONE];

  // number of keys stored in node
  int offset_ = 0;
} NodeType;

class BTree {
 public:
  BTree();
  ~BTree();

  // insert item
  bool Insert(const KeyFieldType& item);

  // dump tree's contents
  void Dump(void);

 private:

  // storage for current node being worked on
  NodeType current_node_;
};
