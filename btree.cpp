//===----------------------------------------------------------------------===//
// BTREE SOURCE
//===----------------------------------------------------------------------===//

#include <iostream>
#include <string.h>
#include <unistd.h>

#include "btree.h"

uint32_t success_count = 0;
uint32_t found_count = 0;
uint32_t out_of_space_count = 0;
uint32_t fail_count = 0;
uint32_t retry_count = 0;

// Key generation
uint32_t max_key_length = 10;

// Thread local timer
thread_local Timer<std::ratio<1,1000000>> timer;

BTree::BTree(const configuration& state){

  // Init
  node_.node_size_ = state.node_size;
  node_.mutable_size_ = state.mutable_size;

  node_.offset_ = 0;

  // Allocate space
  node_.key_ = new KeyFieldType[node_.node_size_];
  node_.slot_length_ = new uint32_t[node_.node_size_];
  node_.visible_ = new bool[node_.node_size_];
  node_.mutable_ = new uint32_t[node_.mutable_size_];

  for(uint32_t mutable_bit_itr = 0; mutable_bit_itr < node_.mutable_size_; mutable_bit_itr++){
    node_.mutable_[mutable_bit_itr] = 0;
  }

  for(uint32_t key_itr = 0; key_itr < node_.node_size_; key_itr++) {
    node_.visible_[key_itr] = false;
  }

  // Allocate space for hybrid

  node_.logical_node_size_ = node_.node_size_ / state.mutable_size;

  node_.keys_ = new KeyFieldType*[state.mutable_size];
  node_.slot_lengths_ = new uint32_t*[state.mutable_size];
  node_.visibles_ = new volatile bool*[state.mutable_size];

  for(uint32_t logical_itr = 0; logical_itr < state.mutable_size; logical_itr++) {
    node_.keys_[logical_itr] = new KeyFieldType[node_.logical_node_size_];
    node_.slot_lengths_[logical_itr] = new uint32_t[node_.logical_node_size_];
    node_.visibles_[logical_itr] = new volatile bool[node_.logical_node_size_];
  }

  for(uint32_t logical_itr = 0; logical_itr < state.mutable_size; logical_itr++) {
      for(uint32_t key_itr = 0; key_itr < node_.logical_node_size_; key_itr++) {
        node_.visibles_[logical_itr][key_itr] = false;
      }
    }

  node_.offsets_ = new uint32_t[state.mutable_size];

  for(uint32_t logical_itr = 0; logical_itr < state.mutable_size; logical_itr++) {
    node_.offsets_[logical_itr] = 0;
  }

}

BTree::~BTree(){

  // Deallocate space
  delete[] node_.key_;
  delete[] node_.slot_length_;
  delete[] node_.visible_;
  delete[] node_.mutable_;

  // Deallocate space for hybrid
  for(uint32_t logical_itr = 0; logical_itr < state.mutable_size; logical_itr++) {
    delete[] node_.keys_[logical_itr];
    delete[] node_.slot_lengths_[logical_itr];
    delete[] node_.visibles_[logical_itr];
  }

  delete[] node_.keys_;
  delete[] node_.slot_lengths_;
  delete[] node_.visibles_;

  delete[] node_.offsets_;

}

uint32_t BTree::InsertOffset(const KeyFieldType* key,
                             uint32_t key_length){

  uint32_t slot_itr = 0;
  uint32_t slot_offset = 0;
  timer.Reset();
  timer.Start();

  while(1){

    // Look up slot to be claimed
    uint32_t claimed_slot_offset = node_.offset_;

    // Check if out of bounds
    if(claimed_slot_offset + key_length >= node_.node_size_){
      out_of_space_count++;
      fail_count++;
      goto finish;
    }
    
    // Search all keys till horizon
    for(; slot_offset < claimed_slot_offset ; ) {
      
      // Wait for key to become visible
      while(node_.visible_[slot_itr] == false){
      }
     
      // Compare key lengths
      // TODO:
      //uint32_t slot_length = node_.slot_lengths_[slot_itr];
      uint32_t slot_length = max_key_length;
      bool key_match = false;
      bool equal_length = (slot_length == key_length);
      if(equal_length == true) {
        // Compare key data
        if(strncmp(node_.key_ + slot_offset, key, key_length) == 0){
          key_match = true;
        }
      }
      
      if(key_match == true){
        found_count++;
        fail_count++;
        goto finish;
      }

      // Go to next slot
      slot_offset += slot_length;
      slot_itr++;
    }
    
    // Claimed offset slot by checking for "stable horizon"
    uint32_t next_slot_offset = claimed_slot_offset + key_length;
    uint32_t cas_status = __sync_bool_compare_and_swap(&node_.offset_, claimed_slot_offset,  next_slot_offset);
    
    // Successful insert
    if(cas_status == true){


      // Copy over key and key_length and make it visible
      memcpy(node_.key_ + claimed_slot_offset, key, key_length);
      // TODO:
      //node_.slot_lengths_[slot_itr] = key_length;
      node_.visible_[slot_itr] = true;
    
      success_count++;
      goto finish;
    }
    // Retry based on new horizon
    else {
      retry_count++;
      continue;
    }

  }

  finish:
  timer.Stop();
  uint32_t duration = timer.GetDuration();
  return duration;
}

uint32_t BTree::InsertMutable(const KeyFieldType* key,
                              uint32_t key_length){

  uint32_t slot_itr = 0;
  uint32_t slot_offset = 0;
  bool key_match = false;
  timer.Reset();
  timer.Start();

  // Hash first character
  uint32_t hash = key[0] % node_.mutable_size_;
  auto logical_slot = node_.mutable_ + hash;

  // Look up slot to be claimed
  uint32_t claimed_slot_offset = node_.offset_;

  // Check if out of bounds
  if(claimed_slot_offset + key_length >= node_.node_size_){
    out_of_space_count++;
    fail_count++;
    goto finish;
  }
  
  // Claim logical space
  while(__sync_bool_compare_and_swap(logical_slot, 0, 1) == false){
  }

  // Claim offset slot
  claimed_slot_offset = __sync_fetch_and_add(&node_.offset_, key_length);
  
  // Check if out of bounds
  if(claimed_slot_offset >= node_.node_size_){
    out_of_space_count++;
    fail_count++;
    goto finish;
  }
  
  // Search all keys till horizon
  for(; slot_offset < claimed_slot_offset ; ) {
    bool visible = node_.visible_[slot_itr];
    // TODO:
    //uint32_t slot_length = node_.slot_lengths_[slot_itr];
    uint32_t slot_length = max_key_length;

    // Check only visible keys
    if(visible == true) {

      // Compare key lengths
      bool equal_length = (slot_length == key_length);
      if(equal_length == true) {
        // Compare key data
        if(strncmp(node_.key_ + slot_offset, key, key_length) == 0){
          key_match = true;
        }
      }

    }

    // Go to next slot
    slot_offset += slot_length;
    slot_itr++;
  }
    
  if(key_match == true){
    found_count++;
    fail_count++;

    // Release logical space if key exists
    *logical_slot = 0;
    // TODO:
    //node_.slot_lengths_[slot_itr] = key_length;
    goto finish;
  }

  // Copy over key and key_length and make it visible
  memcpy(node_.key_ + claimed_slot_offset, key, key_length);
  // TODO:
  //node_.slot_lengths_[slot_itr] = key_length;
  node_.visible_[slot_itr] = true;

  success_count++;

  // Release logical space after completing operation
  *logical_slot = 0;

  finish:
  timer.Stop();
  uint32_t duration = timer.GetDuration();
  return duration;
}

uint32_t BTree::InsertHybrid(const KeyFieldType* key,
                             uint32_t key_length){

  uint32_t slot_itr = 0;
  uint32_t slot_offset = 0;
  timer.Reset();
  timer.Start();

  // Hash first character
  uint32_t hash = key[0] % node_.mutable_size_;

  while(1){

    // Look up slot to be claimed
    uint32_t claimed_slot_offset = node_.offsets_[hash];

    // Check if out of bounds
    if(claimed_slot_offset + key_length >= node_.logical_node_size_){
      out_of_space_count++;
      fail_count++;
      goto finish;
    }

    // Search all keys till horizon
    for(; slot_offset < claimed_slot_offset ; ) {

      // Wait for key to become visible
      while(node_.visibles_[hash][slot_itr] == false){
      }

      // Compare key lengths
      // TODO:
      //uint32_t slot_length = node_.slot_lengths_[slot_itr];
      uint32_t slot_length = max_key_length;
      bool key_match = false;
      bool equal_length = (slot_length == key_length);
      if(equal_length == true) {
        // Compare key data
        if(strncmp(node_.keys_[hash] + slot_offset, key, key_length) == 0){
          key_match = true;
        }
      }

      if(key_match == true){
        found_count++;
        fail_count++;
        goto finish;
      }

      // Go to next slot
      slot_offset += slot_length;
      slot_itr++;
    }

    // Claimed offset slot by checking for "stable horizon"
    uint32_t next_slot_offset = claimed_slot_offset + key_length;
    uint32_t cas_status = __sync_bool_compare_and_swap(&node_.offsets_[hash], claimed_slot_offset,  next_slot_offset);

    // Successful insert
    if(cas_status == true){


      // Copy over key and key_length and make it visible
      memcpy(node_.keys_[hash] + claimed_slot_offset, key, key_length);
      // TODO:
      //node_.slot_lengths_[slot_itr] = key_length;
      node_.visibles_[hash][slot_itr] = true;

      success_count++;
      goto finish;
    }
    // Retry based on new horizon
    else {
      retry_count++;
      continue;
    }

  }

  finish:
  timer.Stop();
  uint32_t duration = timer.GetDuration();
  return duration;
}

void BTree::Dump(void){

  for(uint32_t key_itr = 0;
      key_itr < node_.offset_;
      key_itr++){
    std::cout << node_.key_[key_itr] << " ";
  }
  std::cout << "\n";

}
