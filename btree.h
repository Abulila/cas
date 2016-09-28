//===----------------------------------------------------------------------===//
// BTREE HEADER
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

#include "configuration.h"

typedef uint32_t KeyFieldType;

typedef uint32_t DataFieldType;

extern uint32_t success_count;
extern uint32_t found_count;
extern uint32_t out_of_space_count;
extern uint32_t fail_count;
extern uint32_t retry_count;

typedef struct {
  // # of entries in node
  uint32_t node_size_ = 0;

  // # of mutable bits
  uint32_t mutable_size_ = 0;

  // keys
  KeyFieldType* keys_;

  // fake pointers to child nodes
  DataFieldType* values_;

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
  bool InsertOffset(const KeyFieldType& key);

  // insert item
  bool InsertMutable(const KeyFieldType& key);

  // dump tree's contents
  void Dump(void);

 public:

  // storage for current node being worked on
  NodeType node_;
};
