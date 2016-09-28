//===----------------------------------------------------------------------===//
// BTREE SOURCE
//===----------------------------------------------------------------------===//

#include <iostream>

#include "btree.h"

BTree::BTree(){
  // Nothing to do here!
}

BTree::~BTree(){
  // Nothing to do here!
}

bool BTree::Insert(const KeyFieldType& item){

  current_node_.keys_[current_node_.offset_] = item;
  current_node_.offset_++;

  return true;
}

void BTree::Dump(void){

  for(int key_itr = 0;
      key_itr < current_node_.offset_;
      key_itr++){
    std::cout << current_node_.keys_[key_itr] << " ";
  }
  std::cout << "\n";

}
