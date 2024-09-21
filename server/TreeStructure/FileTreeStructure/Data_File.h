#ifndef RealData_File_H
#define RealData_File_H

#include "../../ExtraFunctions.h"

#include "../../Patterns/SingletonFlyweightPattern.h"

#include "BTree_File.h"

#include "../../ErrorHandler/ErrorThrower.h"

// Y = int | std:string

template<typename Y>
class RealData_File final : public BTree_File<Y, std::shared_ptr<certainData>, Y> {
public:
  std::string rootFile;
  std::string source;
  StringPool &str_pool;

  explicit RealData_File(std::string _source, StringPool &_str_pool) : source(std::move(_source)), str_pool(_str_pool) {
  }

  explicit RealData_File(std::string _source, std::string _rootFile,
                         StringPool &_str_pool) : source(std::move(_source)),
                                                  rootFile(std::move(_rootFile)), str_pool(_str_pool) {
  }

  // TODO: Override, but don't realize
  void insert(Y) override {
  }

  void insert(Y, std::shared_ptr<certainData> &);

  void remove(const Y &) override;

  std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > search(const Y &) override;

  void update(Y, std::shared_ptr<certainData> &);

  std::shared_ptr<Y> load(const std::string &) {
    return nullptr;
  }

  void searchByRange(const Y &, const Y &,
                     std::vector<std::shared_ptr<certainData> > &) const;
};

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
class RealDataNode_File final : public BTreeNode_File<Y, std::shared_ptr<certainData> > {
public:
  bool leaf;
  std::string path, fileName;
  std::vector<std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > > keys;
  std::vector<std::string> childrenFiles;
  StringPool &str_pool;

  explicit RealDataNode_File(const bool _leaf, StringPool &_str_pool) : leaf(_leaf), str_pool(_str_pool) {
  }

  explicit RealDataNode_File(const bool _leaf, std::string _path, std::string _fileName, StringPool &_str_pool) : leaf(_leaf),
    path(std::move(_path)), fileName(std::move(_fileName)), str_pool(_str_pool) {
  }

  // TODO: Override, but don't realize
  void insertNonFull(Y &) override {
  }

  void insertNonFull(Y &, std::shared_ptr<certainData> &);

  void splitChild(int, const std::string &) override;

  std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > search(const Y &) override;

  void update(Y, std::shared_ptr<certainData> &);

  void searchByRange(const Y &, const Y &, std::vector<std::shared_ptr<certainData> > &);

  void saveToFile(const std::string &) const;

  static std::shared_ptr<RealDataNode_File> loadFromFile(const std::string &filename, StringPool &_str_pool);

private:
  void borrowFromNext(const int &) override;

  void borrowFromPrev(const int &) override;

  std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > getPred(const int &) override;

  std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > getSucc(const int &) override;

  void merge(const int &) override;

  void fill(const int &) override;

  void removeFromNonLeaf(const int &) override;

public:
  void remove(const Y &) override;
};

//======================================================================================================================

template<typename Y>
void RealData_File<Y>::insert(Y key, std::shared_ptr<certainData> &data) {
  std::ifstream file(source + "/root.dat", std::ios::binary);

  if (!file) {
    CreateDirectory(source.c_str(), nullptr);

    auto rootNode = std::make_shared<RealDataNode_File<Y> >(true, source, "/root.dat", str_pool);

    rootFile = rootNode->path + rootNode->fileName;

    rootNode->insertNonFull(key, data);
  } else {
    rootFile = source + "/root.dat";

    if (RealDataNode_File<Y>::loadFromFile(rootFile, str_pool)->keys.size() == 2 * T - 1) {
      auto newRootNode = std::make_shared<RealDataNode_File<Y> >(false, source, "/root.dat", str_pool);

      newRootNode->splitChild(0, rootFile);
      newRootNode->saveToFile(rootFile);
    }

    std::shared_ptr<RealDataNode_File<Y> > node = std::move(RealDataNode_File<Y>::loadFromFile(rootFile, str_pool));

    node->insertNonFull(key, data);

    file.close();
  }
}

//----------------------------------------------------------------------------------------------------------------------


template<typename Y>
void RealData_File<Y>::remove(const Y &key) {
  if (!rootFile.empty() && RealDataNode_File<Y>::loadFromFile(rootFile, str_pool)) {
    RealDataNode_File<Y>::loadFromFile(rootFile, str_pool)->remove(key);

    if (RealDataNode_File<Y>::loadFromFile(rootFile, str_pool)->keys.empty())
      deleteDirectory(StringToWString(removeFileName(rootFile, "root.dat")));
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > RealData_File<Y>::search(const Y &key) {
  if (RealDataNode_File<Y>::loadFromFile(source + "/root.dat", str_pool))
    return RealDataNode_File<Y>::loadFromFile(source + "/root.dat", str_pool)->search(key);

  return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void RealData_File<Y>::update(Y key, std::shared_ptr<certainData> &data) {
  if (!rootFile.empty() && RealDataNode_File<Y>::loadFromFile(rootFile, str_pool))
    return RealDataNode_File<Y>::loadFromFile(rootFile, str_pool)->update(key, data);
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void RealData_File<Y>::searchByRange(const Y &minKey, const Y &maxKey,
                                     std::vector<std::shared_ptr<certainData> > &vec) const {
  if (minKey > maxKey)
    return;

  if (!rootFile.empty() && RealDataNode_File<Y>::loadFromFile(rootFile, str_pool)) {
    RealDataNode_File<Y>::loadFromFile(rootFile, str_pool)->searchByRange(minKey, maxKey, vec);
  }
}

//======================================================================================================================

template<typename Y>
void RealDataNode_File<Y>::insertNonFull(Y &key, std::shared_ptr<certainData> &data) {
  int i = static_cast<int>(keys.size() - 1);

  if (leaf) {
    while (i >= 0 && key < keys[i]->key)
      --i;

    keys.emplace(keys.begin() + i + 1,
                 std::make_shared<Data<Y, std::shared_ptr<certainData> > >(std::move(key), data));

    saveToFile(path + fileName);
  } else {
    while (i >= 0 && key < keys[i]->key)
      --i;

    if (loadFromFile(childrenFiles[i + 1], str_pool)->keys.size() == 2 * T - 1) {
      splitChild(i + 1, childrenFiles[i + 1]);
      saveToFile(path + fileName);

      if (key > keys[i]->key)
        ++i;
    }

    loadFromFile(childrenFiles[i + 1], str_pool)->insertNonFull(key);
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void RealDataNode_File<Y>::splitChild(const int i, const std::string &yFile) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 100000);


  auto y = loadFromFile(yFile, str_pool);

  if (y->fileName == "/root.dat") {
    do {
      y->fileName = "/node" + std::to_string(distrib(gen)) + ".dat";
    } while (std::ifstream (path + y->fileName, std::ios::binary)) ;
    childrenFiles.emplace(childrenFiles.begin() + i, y->path + y->fileName);
  }

  std::string z_filename;

  do {
    z_filename = "/node" + std::to_string(distrib(gen)) + ".dat";
  } while (std::ifstream(z_filename, std::ios::binary)) ;

  auto z = std::make_shared<RealDataNode_File>(y->leaf, path, z_filename, str_pool);

  keys.emplace(keys.begin() + i, std::move(y->keys[T - 1]));
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

template<typename Y>
std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > RealDataNode_File<Y>::search(const Y &key) {
  int idx = 0;

  while (idx < keys.size() && key > keys[idx]->key)
    ++idx;

  if (idx != keys.size() && keys[idx]->key == key) {
    Y _key = keys[idx]->key;
    std::shared_ptr<certainData> _data = std::make_shared<certainData>(keys[idx]->data->id, keys[idx]->data->name,
                                                                       keys[idx]->data->extraData);
    return std::move(std::make_shared<Data<Y, std::shared_ptr<certainData> > >(std::move(_key), _data));
  }

  if (!leaf && !childrenFiles[idx].empty())
    return loadFromFile(childrenFiles[idx], str_pool)->search(key);

  return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void RealDataNode_File<Y>::update(Y key, std::shared_ptr<certainData> &data) {
  int idx = 0;

  while (idx < keys.size() && key > keys[idx]->key)
    ++idx;

  if (idx != keys.size() && keys[idx]->key == key) {
    keys[idx]->data->id = data->id;
    keys[idx]->data->name = data->name;
    keys[idx]->data->extraData = data->extraData;

    saveToFile(path + fileName);
    return;
  }

  auto _ = childrenFiles[idx];

  if (!leaf && !childrenFiles[idx].empty())
    loadFromFile(childrenFiles[idx], str_pool)->update(key, data);
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void RealDataNode_File<Y>::searchByRange(const Y &minKey, const Y &maxKey,
                                         std::vector<std::shared_ptr<certainData> > &vec) {
  int idx = 0;

  while (idx < keys.size() && keys[idx]->key < minKey)
    ++idx;

  while (idx < keys.size() && keys[idx]->key >= minKey && keys[idx]->key <= maxKey) {
    std::shared_ptr<certainData> _data = std::make_shared<certainData>(keys[idx]->data->id, keys[idx]->data->name,
                                                                       keys[idx]->data->extraData);
    vec.emplace_back(std::move(_data));

    if (!leaf && childrenFiles.size() > idx && !childrenFiles[idx].empty())
      loadFromFile(childrenFiles[idx], str_pool)->searchByRange(minKey, maxKey, vec);

    ++idx;
  }

  if (!leaf && childrenFiles.size() > idx && !childrenFiles[idx].empty())
    loadFromFile(childrenFiles[idx], str_pool)->searchByRange(minKey, maxKey, vec);
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void RealDataNode_File<Y>::remove(const Y &key) {
  int idx = 0;

  while (idx < keys.size() && keys[idx]->key < key)
    ++idx;

  if (idx < keys.size() && keys[idx]->key == key) {
    if (leaf) {
      keys.erase(keys.begin() + idx);
      saveToFile(path);
    } else {
      removeFromNonLeaf(idx);
    }
  } else {
    if (leaf) {
      std::stringstream ss;
      ss << "Элемент с ключом = \"" << key << "\" не существует!\nПроверьте ключ и повторите попытку\n";

      throw BTreeError(ss.str());
    }

    if (idx != 0)
      --idx;

    int navNum = (key < keys[idx]->key) ? 0 : 1;

    if (loadFromFile(childrenFiles[idx + navNum], str_pool)->keys.size() < T) {
      fill(idx);
      return remove(key);
    }

    loadFromFile(childrenFiles[idx + navNum], str_pool)->remove(key);
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void RealDataNode_File<Y>::removeFromNonLeaf(const int &idx) {
  Y key = keys[idx]->key;

  if (loadFromFile(childrenFiles[idx + 1], str_pool)->keys.size() >= T) {
    std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > replacement = getSucc(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = replacement->data;

    loadFromFile(childrenFiles[idx + 1], str_pool)->remove(replacement->key);
  } else if (loadFromFile(childrenFiles[idx], str_pool)->keys.size() >= T) {
    std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > replacement = getPred(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = replacement->data;

    loadFromFile(childrenFiles[idx + 1], str_pool)->remove(replacement->key);
  } else {
    merge(idx);
    loadFromFile(childrenFiles[idx], str_pool)->remove(key);
  }

  saveToFile(path + fileName);
}

//======================================================================================================================

template<typename Y>
void RealDataNode_File<Y>::saveToFile(const std::string &filename) const {
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file for writing: " << filename << std::endl;
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

template<typename Y>
std::shared_ptr<RealDataNode_File<Y> > RealDataNode_File<Y>::loadFromFile(const std::string &filename, StringPool &_str_pool) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file for reading: " << filename << std::endl;
    return nullptr;
  }

  bool leaf;
  file.read(reinterpret_cast<char *>(&leaf), sizeof(leaf));

  std::string _path;
  deserializeData(file, _path);

  std::string _fileName;
  deserializeData(file, _fileName);

  auto node = std::make_shared<RealDataNode_File>(leaf, _path, _fileName, _str_pool);

  int numKeys;
  file.read(reinterpret_cast<char *>(&numKeys), sizeof(numKeys));

  for (int i = 0; i < numKeys; i++) {
    Y key;
    std::shared_ptr<certainData> RealData_File;

    deserializeData(file, key);
    deserializeData(file, RealData_File, _str_pool);

    node->keys.emplace_back(
      std::make_shared<Data<Y, std::shared_ptr<certainData> > >(std::move(key), std::move(RealData_File)));
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

template<typename Y>
void RealDataNode_File<Y>::borrowFromNext(const int &idx) {
  auto child = loadFromFile(childrenFiles[idx], str_pool);
  auto sibling = loadFromFile(childrenFiles[idx + 1], str_pool);

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

template<typename Y>
void RealDataNode_File<Y>::borrowFromPrev(const int &idx) {
  auto child = loadFromFile(childrenFiles[idx], str_pool);
  auto sibling = loadFromFile(childrenFiles[idx - 1], str_pool);

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

template<typename Y>
void RealDataNode_File<Y>::merge(const int &idx) {
  auto child = loadFromFile(childrenFiles[idx], str_pool);
  auto sibling = loadFromFile(childrenFiles[idx + 1], str_pool);

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

template<typename Y>
std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > RealDataNode_File<Y>::getPred(const int &idx) {
  auto res = loadFromFile(childrenFiles[idx], str_pool);

  while (!res->leaf)
    res = loadFromFile(res->childrenFiles.back(), str_pool);

  return res->keys.back();
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
std::shared_ptr<Data<Y, std::shared_ptr<certainData> > > RealDataNode_File<Y>::getSucc(const int &idx) {
  auto res = loadFromFile(childrenFiles[idx + 1], str_pool);

  while (!res->leaf)
    res = loadFromFile(res->childrenFiles.front(), str_pool);

  return res->keys.front();
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void RealDataNode_File<Y>::fill(const int &idx) {
  if (idx != keys.size() && loadFromFile(childrenFiles[idx + 1], str_pool)->keys.size() >= T) {
    borrowFromNext(idx);
  } else if (idx != 0 && loadFromFile(childrenFiles[idx - 1], str_pool)->keys.size() >= T) {
    borrowFromPrev(idx);
  } else {
    if (idx == keys.size())
      merge(idx - 1);
    else
      merge(idx);
  }
}
#endif //RealData_File_H
