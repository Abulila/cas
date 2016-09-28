//===----------------------------------------------------------------------===//
// BTREE SOURCE
//===----------------------------------------------------------------------===//

#include <iostream>

#include "btree.h"

thread_local uint32_t success_count = 0;

BTree::BTree(){
  current_node_.offset_ = 0;
}

BTree::~BTree(){
  // Nothing to do here!
}

bool BTree::Insert(const KeyFieldType& item){

  uint32_t value_itr = 0;

  while(1){

    // Look up current offset
    uint32_t old_value = current_node_.offset_;

    if(old_value == MAX_KEYS){
      return false;
    }

    // Search for key till horizon
    for(; value_itr < old_value; value_itr++) {
      if(current_node_.keys_[value_itr] == item){
        return false;
      }
    }

    // Claim slot
    uint32_t new_value = old_value + 1;
    uint32_t result = __sync_val_compare_and_swap(&current_node_.offset_, old_value,  new_value);

    // Success
    if(result == old_value){
      current_node_.keys_[old_value] = item;
      success_count++;
      return true;
    }
    // Retry
    else {
      continue;
    }

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
