//===----------------------------------------------------------------------===//
// BTREE SOURCE
//===----------------------------------------------------------------------===//

#include <iostream>

#include "btree.h"

uint32_t success_count = 0;
uint32_t found_count = 0;
uint32_t out_of_space_count = 0;
uint32_t fail_count = 0;
uint32_t retry_count = 0;

BTree::BTree(const configuration& state){

  // Init
  node_.node_size_ = state.node_size;
  node_.mutable_size_ = state.mutable_size;
  node_.offset_ = 0;

  // Allocate space
  node_.keys_ = new uint32_t[node_.node_size_];
  node_.values_ = new uint32_t[node_.node_size_];
  node_.mutable_ = new uint32_t[node_.mutable_size_];

  for(uint32_t mutable_bit_itr = 0;
      mutable_bit_itr < node_.mutable_size_;
      mutable_bit_itr++){
    node_.mutable_[mutable_bit_itr] = 0;
  }

  for(uint32_t key_itr = 0;
      key_itr < node_.node_size_;
      key_itr++) {
    node_.keys_[key_itr] = -1;
  }

}

BTree::~BTree(){

  // Deallocate space
  delete[] node_.keys_;
  delete[] node_.values_;
  delete[] node_.mutable_;

}

bool BTree::InsertOffset(const KeyFieldType& key){

  uint32_t slot_itr = 0;

  while(1){

    // Look up current slot
    uint32_t current_slot = node_.offset_;

    // Check if out of bounds
    if(current_slot >= node_.node_size_){
      out_of_space_count++;
      fail_count++;
      return false;
    }

    // Search all keys till horizon
    for(; slot_itr < current_slot; slot_itr++) {
      if(node_.keys_[slot_itr] == key){
        found_count++;
        fail_count++;
        return false;
      }
    }

    // Claim slot by checking if horizon is stable
    uint32_t claimed_slot = current_slot + 1;
    uint32_t result = __sync_val_compare_and_swap(&node_.offset_, current_slot,  claimed_slot);

    // Successful insert
    if(result == current_slot){
      node_.keys_[current_slot] = key;
      success_count++;
      return true;
    }
    // Retry based on new horizon
    else {
      retry_count++;
      continue;
    }

  }

}

bool BTree::InsertMutable(const KeyFieldType& key){

  uint32_t slot_itr = 0;

  while(1) {

    uint32_t hash = key % node_.mutable_size_;
    uint32_t* logical_slot = node_.mutable_ + hash;

    // Claim logical space
    bool status = __sync_bool_compare_and_swap(logical_slot, 0, 1);

    // Retry
    if(status == false){
      retry_count++;
      continue;
    }

    // Claim slot
    uint32_t claimed_slot = __sync_fetch_and_add(&node_.offset_, 1);

    // Check if out of bounds
    if(claimed_slot >= node_.node_size_){
      out_of_space_count++;
      fail_count++;
      return false;
    }


    // Search all keys till horizon
    for(; slot_itr < claimed_slot; slot_itr++) {
      if(node_.keys_[slot_itr] == key){
        found_count++;
        fail_count++;

        // Release logical space if key exists
        *logical_slot = 0;
        return false;
      }
    }

    // Success
    node_.keys_[claimed_slot] = key;
    success_count++;

    // Release logical space after completing operation
    *logical_slot = 0;

    return true;
  }

}

void BTree::Dump(void){

  for(uint32_t key_itr = 0;
      key_itr < node_.offset_;
      key_itr++){
    std::cout << node_.keys_[key_itr] << " ";
  }
  std::cout << "\n";

}
