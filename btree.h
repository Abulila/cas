//===----------------------------------------------------------------------===//
// BTREE HEADER
//===----------------------------------------------------------------------===//

#include <cstdint>

// max number of keys in a node
const int MAX_KEYS = 11;
const int MAX_KEYS_PLUS_ONE = MAX_KEYS + 1;

// min number of keys in a node
const int MIN_KEYS = 5;

// null pointer
const long NULL_PTR = -1L;

typedef uint32_t KeyFieldType;

typedef uint32_t DataFieldType;

typedef struct {
  KeyFieldType KeyField;
  DataFieldType DataField;
} ItemType;

typedef struct {
  ItemType Key[MAX_KEYS];
  // Fake pointers to child nodes
  long Branch[MAX_KEYS_PLUS_ONE];
  // Number of keys stored in node
  int Count;
} NodeType;

class BTree {
 public:
  BTree();
  ~BTree();

  // insert item
  bool Insert(const ItemType & item);

  // dump tree's contents
  void Dump(void);

 private:

  // number of nodes in the B-tree
  long num_nodes_;

  // number of bytes per node
  int  node_size_;

  // storage for current node being worked on
  NodeType current_node_;
};
