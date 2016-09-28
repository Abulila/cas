//===----------------------------------------------------------------------===//
// MAIN SOURCE
//===----------------------------------------------------------------------===//

#include <iostream>
#include <thread>
#include <vector>

#include "btree.h"
#include "timer.h"

void Insert(BTree *tree){

  for(uint32_t key = 0; key < MAX_KEYS; key++) {
    tree->Insert(key);
  }

}

int main(void){

  uint32_t num_threads = 4;

  BTree tree;
  Timer<> timer;

  std::vector<std::thread> thread_group;

  timer.Start();

  // Launch a group of threads
  for (uint64_t thread_itr = 0;
      thread_itr < num_threads;
      ++thread_itr) {
    thread_group.push_back(std::thread(Insert, &tree));
  }

  // Join the threads with the main thread
  for (uint64_t thread_itr = 0;
      thread_itr < num_threads;
      ++thread_itr) {
    thread_group[thread_itr].join();
  }

  timer.Stop();

  auto duration = timer.GetDuration();

  std::cout << "Duration: " << duration;

  //tree.Dump();

  return 0;
}
