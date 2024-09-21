#ifndef COMMONPOOL_FILE_H
#define COMMONPOOL_FILE_H

#include <utility>

#include "../../ExtraFunctions.h"

#include "SchemaPool_File.h"
#include "BTree_File.h"

#include "../../ErrorHandler/ErrorThrower.h"

using StringNodeForPool_File = Data<std::string, std::string>;

class CommonPool_File final : public BTree_File<std::string, std::string, SchemaPool_File>  {
public:
  std::string rootFile;
  std::string source;

  explicit CommonPool_File(std::string _source) : source(std::move(_source)) {
  }

  explicit CommonPool_File() = default;

  void setSource(const std::string &_source) {
    source = _source;
  }

  void insert(std::string) override;

  void remove(const std::string &) override;

  std::shared_ptr<SchemaPool_File> load(const std::string &key);

  std::shared_ptr<StringNodeForPool_File> search(const std::string &) override;
};

//----------------------------------------------------------------------------------------------------------------------

class CommonPoolNode_File final : public BTreeNode_File<std::string, std::string> {
public:
  bool leaf;
  std::string path, fileName;
  std::vector<std::shared_ptr<StringNodeForPool_File> > keys;
  std::vector<std::string> childrenFiles;

  explicit CommonPoolNode_File(const bool _leaf, std::string _path, std::string _fileName) : leaf(_leaf),
    path(std::move(_path)), fileName(std::move(_fileName)) {
  }

  void insertNonFull(std::string &) override;

  void splitChild(int, const std::string &) override;

  std::shared_ptr<StringNodeForPool_File> search(const std::string &) override;

  void saveToFile(const std::string &) const;

  static std::shared_ptr<CommonPoolNode_File> loadFromFile(const std::string &);

private:
  void borrowFromNext(const int &) override;

  void borrowFromPrev(const int &) override;

  std::shared_ptr<Data<std::string, std::string> > getPred(const int &) override;

  std::shared_ptr<Data<std::string, std::string> > getSucc(const int &) override;

  void merge(const int &) override;

  void fill(const int &) override;

  void removeFromNonLeaf(const int &) override;

public:
  void remove(const std::string &) override;
};

//======================================================================================================================

inline void CommonPool_File::insert(std::string key) {
  std::ifstream file(source + "/root.dat", std::ios::binary);

  if (!file) {
    CreateDirectory(source.c_str(), nullptr);

    auto rootNode = std::make_shared<CommonPoolNode_File>(true, source, "/root.dat");

    rootFile = rootNode->path + rootNode->fileName;

    rootNode->insertNonFull(key);
  } else {
    rootFile = source + "/root.dat";

    auto some = search(key);

    if (some)
      throw BTreeError("Пул с ключом = \"" + key + "\" уже существует!\nПроверьте ключ и повторите попытку\n");

    if (CommonPoolNode_File::loadFromFile(rootFile)->keys.size() == 2 * T - 1) {
      auto newRootNode = std::make_shared<CommonPoolNode_File>(false, source, "/root.dat");

      newRootNode->splitChild(0, rootFile);
      newRootNode->saveToFile(rootFile);
    }

    std::shared_ptr<CommonPoolNode_File> node = CommonPoolNode_File::loadFromFile(rootFile);
    node->insertNonFull(key);

    file.close();
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPool_File::remove(const std::string &key) {
  if (!rootFile.empty() && CommonPoolNode_File::loadFromFile(rootFile))
    CommonPoolNode_File::loadFromFile(rootFile)->remove(key);

  if (CommonPoolNode_File::loadFromFile(rootFile)->keys.empty())
    deleteDirectory(StringToWString(removeFileName(rootFile, "root.dat")));
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<StringNodeForPool_File> CommonPool_File::search(const std::string &key) {
  if (!rootFile.empty() && CommonPoolNode_File::loadFromFile(rootFile))
    return CommonPoolNode_File::loadFromFile(rootFile)->search(key);

  return nullptr;
}

//======================================================================================================================

inline void CommonPoolNode_File::insertNonFull(std::string &key) {
  int i = static_cast<int>(keys.size() - 1);

  if (leaf) {
    while (i >= 0 && key < keys[i]->key)
      --i;

    std::string _path = path;

    keys.emplace(keys.begin() + i + 1, std::make_shared<StringNodeForPool_File>(std::move(key), std::move(_path)));

    saveToFile(path + fileName);
  } else {
    while (i >= 0 && key < keys[i]->key)
      --i;

    if (loadFromFile(childrenFiles[i + 1])->keys.size() == 2 * T - 1) {
      splitChild(i + 1, childrenFiles[i + 1]);
      saveToFile(path + fileName);

      if (key > keys[i + 1]->key)
        ++i;
    }

    loadFromFile(childrenFiles[i + 1])->insertNonFull(key);
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_File::splitChild(const int i, const std::string &yFile) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 100000);

  auto y = loadFromFile(yFile);

  if (y->fileName == "/root.dat") {
    do {
      y->fileName = "/node" + std::to_string(distrib(gen)) + ".dat";
    } while (std::ifstream (path + y->fileName, std::ios::binary)) ;

    childrenFiles.emplace(childrenFiles.begin() + i, y->path + y->fileName);
  }

  std::string z_filename;

  do {
    z_filename = "/node" + std::to_string(distrib(gen)) + ".dat";
  } while (std::ifstream (z_filename, std::ios::binary)) ;

  auto z = std::make_shared<CommonPoolNode_File>(y->leaf, path, z_filename);

  keys.emplace(keys.begin() + i, y->keys[T - 1]);
  childrenFiles.emplace(childrenFiles.begin() + i + 1, z->path + z->fileName);

  z->keys.resize(y->keys.size() - T);
  z->keys.assign(y->keys.begin() + T, y->keys.end());
  y->keys.resize(T - 1);

  if (!y->leaf) {
    z->childrenFiles.assign(y->childrenFiles.begin() + T, y->childrenFiles.end());
    y->childrenFiles.resize(T);
  }

    z->saveToFile(z->path + z->fileName);
  y->saveToFile(y->path + y->fileName);
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<StringNodeForPool_File> CommonPoolNode_File::search(const std::string &key) {
  int idx = 0;

  while (idx < keys.size() && key > keys[idx]->key)
    ++idx;

  if (idx != keys.size() && keys[idx]->key == key)
    return keys[idx];

  if (!leaf && !childrenFiles[idx].empty())
    return loadFromFile(childrenFiles[idx])->search(key);

  return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<SchemaPool_File> CommonPool_File::load(const std::string &key) {
  const std::shared_ptr<StringNodeForPool_File> &data = search(key);

  if (data)
    return std::make_shared<SchemaPool_File>(data->data + '/' + data->key);

  return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_File::remove(const std::string &key) {
  int idx = 0;
  auto _ = keys;
  auto _c = childrenFiles;

  while (idx < keys.size() && keys[idx]->key < key) {
    ++idx;
  }

  if (idx < keys.size() && keys[idx]->key == key) {
    if (leaf) {
      keys.erase(keys.begin() + idx);
      deleteDirectory(StringToWString(path + '/' + key));
      saveToFile(path + fileName);
    } else {
      removeFromNonLeaf(idx);
    }
  } else {
    if (leaf)
      throw BTreeError("Пула с ключом = \"" + key + "\" не существует!\nПроверьте ключ и повторите попытку\n");

    if (idx != 0)
      --idx;

    int navNum = (key < keys[idx]->key) ? 0 : 1;

    if (loadFromFile(childrenFiles[idx + navNum])->keys.size() < T) {
      fill(idx);
      return remove(key);
    }

    loadFromFile(childrenFiles[idx + navNum])->remove(key);
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_File::removeFromNonLeaf(const int &idx) {
  std::string key = keys[idx]->key;

  if (loadFromFile(childrenFiles[idx + 1])->keys.size() >= T) {
    std::shared_ptr<StringNodeForPool_File> replacement = getSucc(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = replacement->data;

    loadFromFile(childrenFiles[idx + 1])->remove(replacement->key);
  } else if (loadFromFile(childrenFiles[idx])->keys.size() >= T) {
    std::shared_ptr<StringNodeForPool_File> replacement = getPred(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = replacement->data;

    loadFromFile(childrenFiles[idx + 1])->remove(replacement->key);
  } else {
    merge(idx);
    loadFromFile(childrenFiles[idx])->remove(key);
  }

  deleteDirectory(StringToWString(path + '/' + key));

  saveToFile(path + fileName);
}

//======================================================================================================================

inline void CommonPoolNode_File::saveToFile(const std::string &filepath) const {
  std::ofstream file(filepath, std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file for writing: " << filepath << std::endl;
    return;
  }

  file.write(reinterpret_cast<const char *>(&leaf), sizeof(leaf));
  serializeData(file, path);
  serializeData(file, fileName);

  const int numKeys = static_cast<int>(keys.size());
  file.write(reinterpret_cast<const char *>(&numKeys), sizeof(numKeys));

  for (const auto &ptr: keys) {
    serializeData(file, ptr->key);
    serializeData(file, ptr->data);
  }

  const int numChildren = static_cast<int>(childrenFiles.size());
  file.write(reinterpret_cast<const char *>(&numChildren), sizeof(numChildren));

  for (const auto &childFile: childrenFiles) {
    int length = static_cast<int>(childFile.length());
    file.write(reinterpret_cast<const char *>(&length), sizeof(length));
    file.write(childFile.c_str(), length);
  }

  file.close();
}

//======================================================================================================================

inline std::shared_ptr<CommonPoolNode_File> CommonPoolNode_File::loadFromFile(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file for reading: " << path << std::endl;
    return nullptr;
  }

  bool leaf;
  file.read(reinterpret_cast<char *>(&leaf), sizeof(leaf));
  std::string _path;
  deserializeData(file, _path);

  std::string _fileName;
  deserializeData(file, _fileName);

  auto node = std::make_shared<CommonPoolNode_File>(leaf, _path, _fileName);

  int numKeys;
  file.read(reinterpret_cast<char *>(&numKeys), sizeof(numKeys));

  for (int i = 0; i < numKeys; i++) {
    std::string key;
    std::string data;

    deserializeData(file, key);
    deserializeData(file, data);

    node->keys.emplace_back(std::make_shared<StringNodeForPool_File>(std::move(key), std::move(data)));
  }

  int numChildren;
  file.read(reinterpret_cast<char *>(&numChildren), sizeof(numChildren));
  node->childrenFiles.resize(numChildren);

  for (int i = 0; i < numChildren; ++i) {
    int length;
    file.read(reinterpret_cast<char *>(&length), sizeof(length));
    node->childrenFiles[i].resize(length);
    file.read(&node->childrenFiles[i][0], length);
  }

  file.close();
  return node;
}

//======================================================================================================================

inline void CommonPoolNode_File::borrowFromNext(const int &idx) {
  auto child = loadFromFile(childrenFiles[idx]);
  auto sibling = loadFromFile(childrenFiles[idx + 1]);

  child->keys.emplace_back(std::move(keys[idx]));
  keys[idx] = std::move(sibling->keys[0]);
  sibling->keys.erase(sibling->keys.begin());

  if (!child->leaf) {
    child->childrenFiles.emplace_back(std::move(sibling->childrenFiles[0]));
    sibling->childrenFiles.erase(sibling->childrenFiles.begin());
  }

  saveToFile(path + fileName);
  child->saveToFile(childrenFiles[idx]);
  sibling->saveToFile(childrenFiles[idx + 1]);
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_File::borrowFromPrev(const int &idx) {
  auto child = loadFromFile(childrenFiles[idx]);
  auto sibling = loadFromFile(childrenFiles[idx - 1]);

  child->keys.emplace(child->keys.begin(), std::move(keys[idx - 1]));
  keys[idx - 1] = std::move(sibling->keys.back());
  sibling->keys.pop_back();

  if (!child->leaf) {
    child->childrenFiles.emplace(child->childrenFiles.begin(), std::move(sibling->childrenFiles.back()));
    sibling->childrenFiles.pop_back();
  }

  saveToFile(path + fileName);
  child->saveToFile(childrenFiles[idx]);
  sibling->saveToFile(childrenFiles[idx - 1]);
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_File::merge(const int &idx) {
  auto child = loadFromFile(childrenFiles[idx]);
  auto sibling = loadFromFile(childrenFiles[idx + 1]);

  child->keys.emplace_back(std::move(keys[idx]));

  for (auto &key: sibling->keys)
    child->keys.emplace_back(std::move(key));

  if (!child->leaf) {
    for (auto &filePath: sibling->childrenFiles)
      child->childrenFiles.emplace_back(std::move(filePath));
  }

  keys.erase(keys.begin() + idx);
  SafeDeleteFile(StringToWString(childrenFiles[idx + 1]));
  childrenFiles.erase(childrenFiles.begin() + idx + 1);

  if (keys.empty()) {
    SafeDeleteFile(StringToWString(childrenFiles[idx]));

    keys = std::move(child->keys);
    childrenFiles = std::move(child->childrenFiles);
    leaf = child->leaf;
  } else {
    child->saveToFile(childrenFiles[idx]);
  }

  saveToFile(path + fileName);
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<StringNodeForPool_File> CommonPoolNode_File::getPred(const int &idx) {
  auto res = loadFromFile(childrenFiles[idx]);

  while (!res->leaf)
    res = loadFromFile(res->childrenFiles.back());

  return res->keys.back();
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<StringNodeForPool_File> CommonPoolNode_File::getSucc(const int &idx) {
  auto res = loadFromFile(childrenFiles[idx + 1]);

  while (!res->leaf)
    res = loadFromFile(res->childrenFiles.front());

  return res->keys.front();
}

//----------------------------------------------------------------------------------------------------------------------

inline void CommonPoolNode_File::fill(const int &idx) {
  if (idx != keys.size() && loadFromFile(childrenFiles[idx + 1])->keys.size() >= T) {
    borrowFromNext(idx);
  } else if (idx != 0 && loadFromFile(childrenFiles[idx - 1])->keys.size() >= T) {
    borrowFromPrev(idx);
  } else {
    if (idx == keys.size())
      merge(idx - 1);
    else
      merge(idx);
  }
}

#endif //COMMONPOOL_FILE_H
