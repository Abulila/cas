//===----------------------------------------------------------------------===//
// BTREE SOURCE
//===----------------------------------------------------------------------===//

#include <iostream>

#include "btree.h"

BTree::BTree(){
  current_node_.offset_ = 0;
}

BTree::~BTree(){
  // Nothing to do here!
}

bool BTree::Insert(const KeyFieldType& item){

  uint32_t old_value = current_node_.offset_;

  if(old_value == MAX_KEYS){
    return false;
  }

  uint32_t new_value = old_value + 1;

  uint32_t result = __sync_val_compare_and_swap(&current_node_.offset_, old_value,  new_value);

  // Success
  if(result == old_value){
    current_node_.keys_[old_value] = item;
    return true;
  }
  // Failure
  else {
    return false;
  }

}

void BTree::Dump(void){

  for(uint32_t key_itr = 0;
      key_itr < current_node_.offset_;
      key_itr++){
    std::cout << current_node_.keys_[key_itr] << " ";
  }
  std::cout << "\n";

}
