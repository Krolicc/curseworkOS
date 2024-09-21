#ifndef COMMONPOOL_MEMORY_H
#define COMMONPOOL_MEMORY_H

#include "../../ExtraFunctions.h"

#include "SchemaPool_Memory.h"
#include "BTree_Memory.h"

#include "../../ErrorHandler/ErrorThrower.h"

class CommonPool_Memory;

using StringNodeForPool_Memory = Data<std::string, std::shared_ptr<SchemaPool_Memory> >;

class CommonPoolNode_Memory final : public BTreeNode_Memory<std::string, std::shared_ptr<SchemaPool_Memory> > {
public:
  bool leaf;
  std::vector<std::shared_ptr<StringNodeForPool_Memory> > keys;
  std::vector<std::shared_ptr<CommonPoolNode_Memory> > children;

  explicit CommonPoolNode_Memory(const bool _leaf) : leaf(_leaf) {
  }

  void insertNonFull(std::string &) override;

  void splitChild(int, std::shared_ptr<CommonPoolNode_Memory> &);

  std::shared_ptr<StringNodeForPool_Memory> search(const std::string &) override;

  void saveToFile(std::ofstream &file) const;

  void createSharedPtr(const std::string &);

private:
  void borrowFromNext(const int &) override;

  void borrowFromPrev(const int &) override;

  std::shared_ptr<StringNodeForPool_Memory> getPred(const int &) override;

  std::shared_ptr<StringNodeForPool_Memory> getSucc(const int &) override;

  void merge(const int &) override;

  void fill(const int &) override;

  void removeFromNonLeaf(const int &) override;

public:
  void remove(const std::string &) override;
};

//----------------------------------------------------------------------------------------------------------------------

class CommonPool_Memory final : public BTree_Memory<std::string, std::shared_ptr<SchemaPool_Memory>,
      SchemaPool_Memory> {
public:
  std::shared_ptr<CommonPoolNode_Memory> root;
  int elemsCount;

  explicit CommonPool_Memory() : root(nullptr), elemsCount(0) {
  }

  void insert(std::string) override;

  void remove(const std::string &) override;

  std::shared_ptr<StringNodeForPool_Memory> search(const std::string &) override;

  std::shared_ptr<SchemaPool_Memory> load(const std::string &);

  void saveToFile(std::ofstream &) const;

  void loadFromFile(std::ifstream &file, StringPool &strPool);
};

//======================================================================================================================

inline void CommonPool_Memory::insert(std::string key) {
  if (!root) {
    root = std::make_shared<CommonPoolNode_Memory>(true);
    root->keys.emplace_back(std::make_unique<StringNodeForPool_Memory>(std::move(key), nullptr));
    ++elemsCount;
  } else {
    auto some = search(key);

    if (some)
      throw BTreeError("Элемент с ключом = \"" + key + "\" уже существует!\nИзмените ключ и повторите попытку\n");

    ++elemsCount;

    if (root->keys.size() == 2 * T - 1) {
      auto newRootNode = std::make_shared<CommonPoolNode_Memory>(false);
      newRootNode->children.emplace_back(root);
      newRootNode->splitChild(0, root);

      int i = 0;
      if (newRootNode->keys[0]->key < key)
        i++;

      newRootNode->children[i]->insertNonFull(key);
      root = newRootNode;
    } else {
      root->insertNonFull(key);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPool_Memory::remove(const std::string &key) {
  if (root != nullptr) {
    if (!root->search(key))
      return;
    root->remove(key);
    --elemsCount;
  } else
    return;

  if (!root->search(key))
    return;

  if (root->keys.empty()) {
    if (root->leaf)
      root = nullptr;
    else
      root = root->children[0];
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<StringNodeForPool_Memory> CommonPool_Memory::search(const std::string &key) {
  if (root != nullptr)
    return root->search(key);

  return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<SchemaPool_Memory> CommonPool_Memory::load(const std::string &key) {
  if (root != nullptr) {
    root->createSharedPtr(key);

    auto _data = search(key);

    if (_data)
      return _data->data;
  }

  return nullptr;
}

//======================================================================================================================

inline void CommonPoolNode_Memory::insertNonFull(std::string &key) {
  int i = static_cast<int>(keys.size() - 1);

  while (i >= 0 && key < keys[i]->key)
    --i;

  if (leaf) {
    keys.emplace(keys.begin() + i + 1, std::make_unique<StringNodeForPool_Memory>(std::move(key), nullptr));
  } else {
    if (children[i + 1]->keys.size() == 2 * T - 1) {
      splitChild(i, children[i + 1]);
      if (key > keys[i]->key)
        ++i;
    }

    children[i + 1]->insertNonFull(key);
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_Memory::splitChild(const int i, std::shared_ptr<CommonPoolNode_Memory> &child) {
  auto sibling = std::make_unique<CommonPoolNode_Memory>(child->leaf);

  keys.insert(keys.begin() + i, std::move(child->keys[T - 1]));

  sibling->keys.resize(child->keys.size() - T);
  std::move(child->keys.begin() + T, child->keys.end(), sibling->keys.begin());
  child->keys.resize(T - 1);

  if (!child->leaf) {
    sibling->children.assign(child->children.begin() + T, child->children.end());
    child->children.resize(T);
  }

  children.emplace(children.begin() + i + 1, std::move(sibling));
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<StringNodeForPool_Memory> CommonPoolNode_Memory::search(const std::string &key) {
  int idx = 0;

  while (idx < keys.size() && key > keys[idx]->key)
    ++idx;

  if (idx != keys.size() && keys[idx] && keys[idx]->key == key) {
    return keys[idx];
  }

  if (!leaf && !children[idx]->keys.empty())
    return children[idx]->search(key);

  return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_Memory::createSharedPtr(const std::string &key) {
  int idx = 0;

  while (idx < keys.size() && key > keys[idx]->key)
    ++idx;

  if (idx != keys.size() && keys[idx] && keys[idx]->key == key) {
    if (!keys[idx]->data)
      keys[idx]->data = std::make_shared<SchemaPool_Memory>();
  }

  if (!leaf && !children[idx]->keys.empty())
    children[idx]->createSharedPtr(key);
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_Memory::remove(const std::string &key) {
  int idx = 0;

  while (idx < keys.size() && keys[idx]->key < key)
    ++idx;

  if (idx < keys.size() && keys[idx]->key == key) {
    if (leaf) {
      keys.erase(keys.begin() + idx);
    } else {
      children[idx]->remove(key);
    }
  } else {
    if (leaf)
      throw BTreeError("Пула с ключом = \"" + key + "\" не существует!\nПроверьте ключ и повторите попытку\n");

    if (children[idx]->keys.size() < T)
      fill(idx);

    if (children.empty())
      remove(key);
    else if (idx == keys.size())
      children[idx - 1]->remove(key);
    else
      children[idx]->remove(key);
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_Memory::removeFromNonLeaf(const int &idx) {
  std::string key = keys[idx]->key;

  if (children[idx + 1]->keys.size() >= T) {
    std::shared_ptr<StringNodeForPool_Memory> replacement = getSucc(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = replacement->data;

    children[idx + 1]->remove(replacement->key);
  } else if (children[idx]->keys.size() >= T) {
    std::shared_ptr<StringNodeForPool_Memory> replacement = getPred(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = replacement->data;

    children[idx + 1]->remove(replacement->key);
  } else {
    merge(idx);
    children[idx]->remove(key);
  }
}

//======================================================================================================================

inline void CommonPool_Memory::saveToFile(std::ofstream &file) const {
  if (root != nullptr) {
    file << elemsCount << '\n';
    root->saveToFile(file);
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_Memory::saveToFile(std::ofstream &file) const {
  for (const auto &el: keys) {
    file << "  " << el->key << " ";
    if (el->data == nullptr)
      file << "0";
    else
      file << el->data->elemsCount;
    file << '\n';
    if (el->data != nullptr)
      el->data->saveToFile(file);
  }

  if (!leaf) {
    for (const auto &el: children)
      el->saveToFile(file);
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPool_Memory::loadFromFile(std::ifstream &file, StringPool &strPool) {
  int _elemsCount;

  file >> _elemsCount;

  std::string name;

  for (int i = 0; i < _elemsCount; ++i) {
    file >> name;
    insert(name);
    load(name);
    auto pull = search(name);
    pull->data->loadFromFile(file, strPool);
    if (pull->data->root == nullptr)
      pull->data = nullptr;
  }
}

//======================================================================================================================

inline void CommonPoolNode_Memory::borrowFromNext(const int &idx) {
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

inline void CommonPoolNode_Memory::borrowFromPrev(const int &idx) {
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

inline void CommonPoolNode_Memory::merge(const int &idx) {
  auto child = children[idx];
  auto sibling = children[idx + 1];

  child->keys.emplace_back(std::move(keys[idx]));

  for (auto &key: sibling->keys)
    child->keys.emplace_back(std::move(key));

  if (!child->leaf) {
    for (auto &filePath: sibling->children)
      child->children.emplace_back(std::move(filePath));
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

inline std::shared_ptr<StringNodeForPool_Memory> CommonPoolNode_Memory::getPred(const int &idx) {
  auto res = children[idx];

  while (!res->leaf)
    res = res->children.back();

  return res->keys.back();
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<StringNodeForPool_Memory> CommonPoolNode_Memory::getSucc(const int &idx) {
  auto res = children[idx + 1];

  while (!res->leaf)
    res = res->children.front();

  return res->keys.front();
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_Memory::fill(const int &idx) {
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

#endif //COMMONPOOL_MEMORY_H
