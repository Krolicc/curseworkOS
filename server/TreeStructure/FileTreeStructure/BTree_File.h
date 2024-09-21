#ifndef BTREE_FILE_H
#define BTREE_FILE_H

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
class BTreeNode_File : public BTreeNode<Y, U> {
public:
  std::string path;
  std::vector<std::string> childrenFiles;

  virtual void splitChild(int, const std::string &) = 0;
};

template<typename Y, typename U, typename O>
class BTree_File : public BTree<Y, U, O> {
public:
  std::string rootFile;
  std::string source;
};

#endif //BTREE_FILE_H
