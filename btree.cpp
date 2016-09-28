//===----------------------------------------------------------------------===//
// BTREE SOURCE
//===----------------------------------------------------------------------===//

#include <iostream>

#include "btree.h"

uint32_t success_count = 0;

BTree::BTree(){
  current_node_.offset_ = 0;

  for(uint32_t mutable_bit_itr = 0;
      mutable_bit_itr < NUM_MUTABLE_BITS;
      mutable_bit_itr++){
    current_node_.mutable_[mutable_bit_itr] = 0;
  }

  for(uint32_t key_itr = 0;
      key_itr < MAX_KEYS;
      key_itr++) {
    current_node_.keys_[key_itr] = 0;
  }

}

BTree::~BTree(){
  // Nothing to do here!
}

bool BTree::InsertOffset(const KeyFieldType& item){

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

bool BTree::InsertMutable(const KeyFieldType& item){

  uint32_t value_itr = 0;

  while(1) {

    // Claim slot
    uint32_t slot = __sync_fetch_and_add(&current_node_.offset_, 1);

    // Release logical space
    if(slot >= MAX_KEYS){
      return false;
    }

    uint32_t hash = item % NUM_MUTABLE_BITS;
    int* logical_slot = current_node_.mutable_ + hash;

    // Claim logical space
    bool status = __sync_bool_compare_and_swap(logical_slot, 0, 1);

    // Retry
    if(status == false){
      continue;
    }

    // Search for key till horizon
    for(; value_itr < slot; value_itr++) {
      if(current_node_.keys_[value_itr] == item){
        // Release logical space
        *logical_slot = 0;
        return false;
      }
    }

    // Success
    current_node_.keys_[slot] = item;
    success_count++;

    // Release logical space
    *logical_slot = 0;

    return true;
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
