#ifndef BTREE_H
#define BTREE_H

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

#include "certainData.h"

constexpr int T = 3;

template<typename Y, typename U>
struct Data {
  Y key;
  U data;

  explicit Data(Y &&_key, U _data): key(std::forward<Y>(_key)), data(_data) {
  }
};

template<typename Y, typename U>
class BTreeNode {
public:
  bool leaf{};
  std::vector<std::shared_ptr<Data<Y, U> > > keys;

  virtual ~BTreeNode() = default;

  virtual void remove(const Y &) = 0;

private:
  virtual void insertNonFull(Y &) = 0;

  virtual void removeFromNonLeaf(const int &) = 0;

  virtual void borrowFromNext(const int &) = 0;

  virtual void borrowFromPrev(const int &) = 0;

  virtual void merge(const int &) = 0;

  virtual void fill(const int &) = 0;

  virtual std::shared_ptr<Data<Y, U> > search(const Y &) = 0;

  virtual std::shared_ptr<Data<Y, U> > getPred(const int &) = 0;

  virtual std::shared_ptr<Data<Y, U> > getSucc(const int &) = 0;
};

template<typename Y, typename U, typename O>
class BTree {
public:
  virtual ~BTree() = default;

  virtual void insert(Y) = 0;

  virtual void remove(const Y &) = 0;


  std::shared_ptr<O> load(const std::string &) = delete;


  void update(const std::string &) = delete;

  void printByOne(const std::string &, const std::string &) = delete;

  void printByRange(const std::string &, const std::string &, const std::string &) const = delete;


  void saveToFile(std::ofstream &) = delete;

  void loadFromFile(std::ifstream &) = delete;

private:
  virtual std::shared_ptr<Data<Y, U> > search(const Y &) = 0;
};
#endif //BTREE_H
