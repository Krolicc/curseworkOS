#ifndef BTREE_MEMORY_H
#define BTREE_MEMORY_H

#include <iostream>
#include <sstream>
#include <fstream>

#include <string>
#include <cstring>

#include <utility>
#include <memory>
#include <random>
#include <variant>
#include <algorithm>
#include <type_traits>

#include <vector>
#include <unordered_map>

#include <windows.h>

#include "../../BaseTree/BTree.h"

template<typename Y, typename U>
class BTreeNode_Memory : public BTreeNode<Y, U>{
public:
  std::vector<std::shared_ptr<BTreeNode_Memory>> children;
};

template<typename Y, typename U, typename O>
class BTree_Memory : public BTree<Y, U, O>{
public:
  std::shared_ptr<BTreeNode_Memory<Y, U> > root;
  int elemsCount;
};

#endif //BTREE_MEMORY_H
