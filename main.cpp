//===----------------------------------------------------------------------===//
// MAIN SOURCE
//===----------------------------------------------------------------------===//

#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <getopt.h>

#include "btree.h"
#include "timer.h"
#include "configuration.h"

// Configuration
configuration state;

void Usage(FILE *out) {
  fprintf(out,
          "Command line options : ycsb <options> \n"
          "   -h --help              :  Print help message \n"
          "   -s --synch-mode        :  synch mode \n"
          "   -k --node-size         :  # of node entries \n"
          "   -m --mutable-size      :  # of mutable entries \n"
          "   -t --thread-count      :  # of threads \n"
          "   -l --loop-count        :  # of loops \n"
          "   -o --op-count          :  # of ops \n"
  );
}

static struct option opts[] = {
    { "synch-mode", optional_argument, NULL, 's'},
    { "node-size", optional_argument, NULL, 'k'},
    { "mutable-size", optional_argument, NULL, 'm'},
    { "thread-count", optional_argument, NULL, 't'},
    { "loop-count", optional_argument, NULL, 'l'},
    { "op-count", optional_argument, NULL, 'o'},
    { NULL, 0, NULL, 0}
};

std::string SynchModeTypeToString(SynchModeType synch_mode_type){
  switch(synch_mode_type){
    case SYNCH_MODE_TYPE_OFFSET:
      return "OFFSET";
      break;

    case SYNCH_MODE_TYPE_MUTABLE:
      return "MUTABLE";
      break;

    default:
    case SYNCH_MODE_TYPE_INVALID:
      break;
  }

  return "INVALID";
}

void ValidateSynchModeType(const configuration &state) {
  if (state.synch_mode_type != SYNCH_MODE_TYPE_OFFSET &&
      state.synch_mode_type != SYNCH_MODE_TYPE_MUTABLE) {
    printf("Invalid synch_mode_type : %s\n", SynchModeTypeToString(state.synch_mode_type).c_str());
    exit(EXIT_FAILURE);
  }

  printf("%s : %s\n", "index type", SynchModeTypeToString(state.synch_mode_type).c_str());
}

void ValidateNodeSize(const configuration &state) {
  if (state.node_size <= 0) {
    printf("Invalid node_size :: %d\n", state.node_size);
    exit(EXIT_FAILURE);
  }

  printf("%s : %d\n", "node_size", state.node_size);
}

void ValidateMutableSize(const configuration &state) {
  if (state.mutable_size <= 0) {
    printf("Invalid mutable_size :: %d\n", state.mutable_size);
    exit(EXIT_FAILURE);
  }

  printf("%s : %d\n", "mutable_size", state.mutable_size);
}

void ValidateThreadCount(const configuration &state) {
  if (state.thread_count <= 0) {
    printf("Invalid thread_count :: %d\n", state.thread_count);
    exit(EXIT_FAILURE);
  }

  printf("%s : %d\n", "thread_count", state.thread_count);
}

void ValidateLoopCount(const configuration &state) {
  if (state.loop_count <= 0) {
    printf("Invalid loop_count :: %d\n", state.loop_count);
    exit(EXIT_FAILURE);
  }

  printf("%s : %d\n", "loop_count", state.loop_count);
}

void ValidateOpCount(const configuration &state) {
  if (state.op_count <= 0) {
    printf("Invalid op_count :: %d\n", state.op_count);
    exit(EXIT_FAILURE);
  }

  printf("%s : %d\n", "op_count", state.op_count);
}

void ParseArguments(int argc, char *argv[], configuration &state) {
  // Default Values
  state.synch_mode_type = SYNCH_MODE_TYPE_OFFSET;
  state.node_size = 1000;
  state.mutable_size = 64;
  state.thread_count = 4;
  state.loop_count = 1000;
  state.op_count = state.node_size/state.thread_count;

  // Parse args
  while (1) {
    int idx = 0;
    int c = getopt_long(argc, argv, "hs:k:m:o:t:l:", opts, &idx);

    if (c == -1) break;

    switch (c) {
      case 's':
        state.synch_mode_type = (SynchModeType) atoi(optarg);
        break;
      case 'k':
        state.node_size = atoi(optarg);
        break;
      case 'm':
        state.mutable_size = atoi(optarg);
        break;
      case 'o':
        state.op_count = atoi(optarg);
        break;
      case 't':
        state.thread_count = atoi(optarg);
        break;
      case 'l':
        state.loop_count = atoi(optarg);
        break;

      case 'h':
        Usage(stderr);
        exit(EXIT_FAILURE);
        break;

      default:
        fprintf(stderr, "\nUnknown option: -%c-\n", c);
        Usage(stderr);
        exit(EXIT_FAILURE);
        break;
    }
  }

  // Print configuration
  ValidateSynchModeType(state);
  ValidateNodeSize(state);
  ValidateThreadCount(state);
  ValidateLoopCount(state);
  ValidateMutableSize(state);
  ValidateOpCount(state);

  printf("----------------------------------\n\n");

}

std::vector<uint32_t> key_lengths;
std::vector<uint32_t> key_offsets;

void InsertOffset(BTree *tree, uint32_t op_count, uint32_t thread_id){

  for(uint32_t op_itr = 0; op_itr < op_count; op_itr++) {
    auto key_itr = op_itr * thread_id;

    auto key_offset = key_offsets[key_itr];
    auto key_length = key_lengths[key_itr];

    KeyFieldType* key = key_data + key_offset;
    tree->InsertOffset(key, key_length);
  }

}

void InsertMutable(BTree *tree, uint32_t op_count, uint32_t thread_id){

  for(uint32_t op_itr = 0; op_itr < op_count; op_itr++) {
    auto key_itr = op_itr * thread_id;

    auto key_offset = key_offsets[key_itr];
    auto key_length = key_lengths[key_itr];

    KeyFieldType* key = key_data + key_offset;
    tree->InsertMutable(key, key_length);
  }

}

int main(int argc, char **argv) {


  // Parse arguments
  ParseArguments(argc, argv, state);

  uint32_t thread_count = state.thread_count;
  uint32_t loop_count = state.loop_count;
  uint32_t op_count = state.op_count;

  Timer<> timer;
  std::vector<std::thread> thread_group;

  // Build key length and offset distribution
  uint32_t key_count = op_count * (thread_count + 1);
  for(uint32_t key_itr = 0; key_itr < key_count; ++key_itr){
    uint32_t key_length = 1 + rand() % max_key_length;
    uint32_t key_offset = rand() % max_offset_length;

    key_lengths.push_back(key_length);
    key_offsets.push_back(key_offset);
  }

  for(uint32_t loop_itr = 0; loop_itr < loop_count; ++loop_itr) {

    thread_group.clear();

    // Construct btree instance
    BTree tree(state);

    // Start timer
    timer.Start();

    // Launch a group of threads
    for (uint32_t thread_itr = 0; thread_itr < thread_count; ++thread_itr) {

      if(state.synch_mode_type == SYNCH_MODE_TYPE_OFFSET) {
        thread_group.push_back(std::thread(InsertOffset,
                                           &tree,
                                           op_count,
                                           thread_itr + 1));
      }
      else if(state.synch_mode_type == SYNCH_MODE_TYPE_MUTABLE) {
        thread_group.push_back(std::thread(InsertMutable,
                                           &tree,
                                           op_count,
                                           thread_itr + 1));
      }

    }

    // Join the threads with the main thread
    for (uint64_t thread_itr = 0;
        thread_itr < thread_count;
        ++thread_itr) {
      thread_group[thread_itr].join();
    }

    // Stop timer
    timer.Stop();

    /*
    auto start = tree.current_node_.keys_;
    auto end = start + tree.current_node_.offset_;
    std::sort(start, end);
    auto unique = std::unique(start, end);
    auto contains_duplicates = (unique != end);
    assert(contains_duplicates == false);
    */
  }

  auto duration = timer.GetDuration();

  std::cout << "Duration           : " << duration << "\n";
  std::cout << "Success count      : " << success_count/loop_count << "\n";
  std::cout << "Found count        : " << found_count/loop_count << "\n";
  std::cout << "Out of space count : " << out_of_space_count/loop_count << "\n";
  std::cout << "Fail count         : " << fail_count/loop_count << "\n";
  std::cout << "Retry count        : " << retry_count/loop_count << "\n";

  //tree.Dump();

  return 0;
}
