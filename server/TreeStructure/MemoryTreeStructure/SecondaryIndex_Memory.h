#ifndef SECONDARYINDEX_MEMORY_H
#define SECONDARYINDEX_MEMORY_H

#include "../../ExtraFunctions.h"

#include "BTree_Memory.h"

#include "../../ErrorHandler/ErrorThrower.h"

// Y = int | std:string

template<typename Y>
class SecIndexNode_Memory final {
public:
  bool leaf;
  std::vector<std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > > keys;
  std::vector<std::shared_ptr<SecIndexNode_Memory> > children{};

  explicit SecIndexNode_Memory(const bool _leaf) : leaf(_leaf) {
  }

  void insertNonFull(Y &, std::shared_ptr<certainData> &);

  void splitChild(int, std::shared_ptr<SecIndexNode_Memory> &);

  std::vector<std::shared_ptr<certainData> > search(const Y &);

  void update(Y, std::shared_ptr<certainData> &);

  void searchByRange(const Y &, const Y &, std::vector<std::shared_ptr<certainData> > &);

private:
  void borrowFromNext(const int &);

  void borrowFromPrev(const int &);

  std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > getPred(const int &);

  std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > getSucc(const int &);

  void merge(const int &);

  void fill(const int &);

  void removeFromNonLeaf(const int &, const int &);

public:
  void remove(const Y &, const int &);
};

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
class SecIndex_Memory final {
public:
  std::shared_ptr<SecIndexNode_Memory<Y> > root;
  int elemsCount;

  SecIndex_Memory() : root(nullptr), elemsCount(0) {
  }

  void insert(Y, std::shared_ptr<certainData> &);

  void remove(const Y &, const int &);

  std::vector<std::shared_ptr<certainData> > search(const Y &);

  void update(Y, std::shared_ptr<certainData> &);

  void searchByRange(const Y &, const Y &,
                     std::vector<std::shared_ptr<certainData> > &) const;

  void saveToFile(std::ofstream &) const {
  }

  std::shared_ptr<SecIndexNode_Memory<Y> > load(const std::string &) {
    return nullptr;
  }

private:
  void insertNonFull(Y &, std::shared_ptr<certainData> &);
};

//======================================================================================================================

template<typename Y>
void SecIndex_Memory<Y>::insert(Y key, std::shared_ptr<certainData> &data) {
  if (!root) {
    root = std::make_shared<SecIndexNode_Memory<Y> >(true);

    root->insertNonFull(key, data);
    ++elemsCount;
  } else {
    ++elemsCount;

    if (root->keys.size() == 2 * T - 1) {
      auto newRootNode = std::make_shared<SecIndexNode_Memory<Y> >(false);

      newRootNode->children.emplace_back(root);
      newRootNode->splitChild(0, root);

      int i = 0;
      if (newRootNode->keys[0]->key < key)
        i++;

      newRootNode->children[i]->insertNonFull(key, data);
      root = newRootNode;
    } else {
      insertNonFull(key, data);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndex_Memory<Y>::remove(const Y &key, const int &id) {
  if (root != nullptr) {
    if (search(key).empty())
      return;

    root->remove(key, id);

    --elemsCount;
  } else
    return;

  if (root->keys.empty()) {
    if (root->leaf)
      root = nullptr;
    else
      root = root->children[0];
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
std::vector<std::shared_ptr<certainData> > SecIndex_Memory<Y>::search(const Y &key) {
  if (root != nullptr)
    return root->search(key);

  return {};
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndex_Memory<Y>::update(Y key, std::shared_ptr<certainData> &data) {
  if (root != nullptr)
    root->update(key, data);
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndex_Memory<Y>::searchByRange(const Y &minKey, const Y &maxKey,
                                       std::vector<std::shared_ptr<certainData> > &vec) const {
  if (minKey > maxKey)
    throw BTreeError("Поменяте границы диапазона местами!\n После повторите попытку.\n");

  if (root != nullptr) {
    root->searchByRange(minKey, maxKey, vec);
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndex_Memory<Y>::insertNonFull(Y &key, std::shared_ptr<certainData> &data) {
  root->insertNonFull(key, data);
}

//======================================================================================================================

template<typename Y>
void SecIndexNode_Memory<Y>::insertNonFull(Y &key, std::shared_ptr<certainData> &data) {
  int i = static_cast<int>(keys.size() - 1);

  while (i >= 0 && key < keys[i]->key)
    --i;

  if (i >= 0 && key == keys[i]->key) {
    keys[i]->data.emplace_back(data);
    return;
  }

  if (leaf) {
    std::vector<std::shared_ptr<certainData> > _;
    _.emplace_back(data);
    keys.emplace(keys.begin() + i + 1,
                 std::make_shared<Data<Y, std::vector<std::shared_ptr<certainData> > > >(
                   std::move(key), std::move(_)));
  } else {
    if (children[i + 1]->keys.size() == 2 * T - 1) {
      splitChild(i, children[i + 1]);

      if (key > keys[i]->key)
        ++i;
    }

    children[i + 1]->insertNonFull(key, data);
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_Memory<Y>::splitChild(const int i, std::shared_ptr<SecIndexNode_Memory> &child) {
  auto sibling = std::make_shared<SecIndexNode_Memory>(child->leaf);

  keys.insert(keys.begin() + i, std::move(child->keys[T - 1]));
  children.emplace(children.begin() + i + 1, sibling);

  sibling->keys.resize(child->keys.size() - T);
  std::move(child->keys.begin() + T, child->keys.end(), sibling->keys.begin());
  child->keys.resize(T - 1);

  if (!child->leaf) {
    sibling->children.assign(child->children.begin() + T, child->children.end());
    child->children.resize(T);
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
std::vector<std::shared_ptr<certainData> > SecIndexNode_Memory<Y>::search(const Y &key) {
  int idx = 0;

  while (idx < keys.size() && key > keys[idx]->key)
    ++idx;

  if (idx != keys.size() && keys[idx]->key == key)
    return keys[idx]->data;

  if (!leaf && children[idx])
    return children[idx]->search(key);

  return {};
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_Memory<Y>:: update(Y key, std::shared_ptr<certainData> &data) {
  int idx = 0;

  while (idx < keys.size() && key > keys[idx]->key)
    ++idx;

  if (idx != keys.size() && keys[idx]->key == key) {
    for (int i = 0; i < keys[idx]->data.size(); ++i) {
      if (keys[idx]->data[i]->id == data->id) {
        keys[idx]->data[i] = data;
        break;
      }
    }

    return;
  }

  if (!leaf && children[idx])
    children[idx]->update(key, data);
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_Memory<Y>::searchByRange(const Y &minKey, const Y &maxKey,
                                           std::vector<std::shared_ptr<certainData> > &vec) {
  int idx = 0;

  while (idx < keys.size() && keys[idx]->key < minKey)
    ++idx;

  while (idx < keys.size() && keys[idx]->key >= minKey && keys[idx]->key <= maxKey) {
    for (int i = 0; i < keys[idx]->data.size(); ++i) {
      vec.emplace_back(keys[idx]->data[i]);
    }

    if (!leaf && children.size() > idx && children[idx])
      children[idx]->searchByRange(minKey, maxKey, vec);

    ++idx;
  }

  if (!leaf && children.size() > idx && children[idx])
    children[idx]->searchByRange(minKey, maxKey, vec);
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_Memory<Y>::remove(const Y &key, const int &id) {
  int idx = 0;

  while (idx < keys.size() && keys[idx]->key < key)
    ++idx;

  if (idx < keys.size() && keys[idx]->key == key) {
    if (leaf || keys[idx]->data.size() > 1) {
      for (int i = 0; i < keys[idx]->data.size(); ++i) {
        if (keys[idx]->data[i]->id == id) {
          keys[idx]->data.erase(keys[idx]->data.begin() + id);
          break;
        }
      }
    } else {
      children[idx]->removeFromNonLeaf(idx, id);
    }
  } else {
    if (leaf) {
      std::stringstream ss;
      ss << "Элемент с ключом = \"" << key << "\" не существует!\nПроверьте ключ и повторите попытку\n";

      throw BTreeError(ss.str());
    }

    if (children[idx]->keys.size() < T)
      fill(idx);

    if (children.empty())
      remove(key, id);
    else if (idx == keys.size())
      children[idx - 1]->remove(key, id);
    else
      children[idx]->remove(key, id);
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_Memory<Y>::removeFromNonLeaf(const int &idx, const int &id) {
  Y key = keys[idx]->key;

  if (children[idx + 1]->keys.size() >= T) {
    std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > replacement = getSucc(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = std::move(replacement->data);

    children[idx + 1]->remove(replacement->key, id);
  } else if (children[idx]->keys.size() >= T) {
    std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > replacement = getPred(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = std::move(replacement->data);

    children[idx + 1]->remove(replacement->key, id);
  } else {
    merge(idx);
    children[idx]->remove(key, id);
  }
}

//======================================================================================================================

template<typename Y>
void SecIndexNode_Memory<Y>::borrowFromNext(const int &idx) {
  auto child = children[idx];
  auto sibling = children[idx + 1];

  child->keys.emplace_back(std::move(keys[idx]));
  keys[idx] = std::move(sibling->keys[0]);
  sibling->keys.erase(sibling->keys.begin());

  if (!child->leaf) {
    child->children.emplace_back(std::move(sibling->children[0]));
    sibling->children.erase(sibling->children.begin());
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_Memory<Y>::borrowFromPrev(const int &idx) {
  auto child = children[idx];
  auto sibling = children[idx - 1];

  child->keys.emplace(child->keys.begin(), std::move(keys[idx - 1]));
  keys[idx - 1] = std::move(sibling->keys.back());
  sibling->keys.pop_back();

  if (!child->leaf) {
    child->children.emplace(child->children.begin(), std::move(sibling->children.back()));
    sibling->children.pop_back();
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_Memory<Y>::merge(const int &idx) {
  auto child = children[idx];
  auto sibling = children[idx + 1];

  child->keys.emplace_back(std::move(keys[idx]));

  for (auto &key: sibling->keys)
    child->keys.emplace_back(std::move(key));

  if (!child->leaf) {
    for (auto &siblingChild: sibling->children)
      child->children.emplace_back(std::move(siblingChild));
  }

  keys.erase(keys.begin() + idx);
  children.erase(children.begin() + idx + 1);

  if (keys.empty()) {
    keys = std::move(child->keys);
    children = std::move(child->children);
    leaf = child->leaf;
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > SecIndexNode_Memory<Y>::getPred(const int &idx) {
  auto res = children[idx];

  while (!res->leaf)
    res = res->children.back();

  return res->keys.front();
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > SecIndexNode_Memory<Y>::getSucc(const int &idx) {
  auto res = children[idx + 1];

  while (!res->leaf)
    res = res->children.front();

  return res->keys.back();
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_Memory<Y>::fill(const int &idx) {
  if (idx != keys.size() && children[idx + 1]->keys.size() >= T) {
    borrowFromNext(idx);
  } else if (idx != 0 && children[idx - 1]->keys.size() >= T) {
    borrowFromPrev(idx);
  } else {
    if (idx == keys.size())
      merge(idx - 1);
    else
      merge(idx);
  }
}

#endif //SECONDARYINDEX_MEMORY_H
