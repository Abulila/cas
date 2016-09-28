//===----------------------------------------------------------------------===//
// MAIN SOURCE
//===----------------------------------------------------------------------===//

#include <iostream>

#include "btree.h"

int main(void){

  BTree tree;

  tree.Insert(1);
  tree.Insert(2);
  tree.Insert(3);

  tree.Dump();

  return 0;
}
