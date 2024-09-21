#ifndef SECONDARYINDEX_FILE_H
#define SECONDARYINDEX_FILE_H

#include "../../ExtraFunctions.h"

#include "../../Patterns/SingletonFlyweightPattern.h"

#include "BTree_File.h"

#include "../../ErrorHandler/ErrorThrower.h"

// Y = int | std:string

template<typename Y>
class SecIndex_File final {
public:
  std::string rootFile;
  std::string source;
  StringPool &str_pool;

  explicit SecIndex_File(std::string _source, StringPool &_str_pool) : source(std::move(_source)), str_pool(_str_pool) {
  }

  explicit SecIndex_File(std::string _source, std::string _rootFile,
                         StringPool &_str_pool) : source(std::move(_source)),
                                                  rootFile(std::move(_rootFile)), str_pool(_str_pool) {
  }

  void insert(Y, std::shared_ptr<certainData> &);

  void remove(const Y &, const int &);

  std::vector<std::shared_ptr<certainData> > search(const Y &);

  void update(Y, std::shared_ptr<certainData> &);

  std::shared_ptr<Y> load(const std::string &) {
    return nullptr;
  }

  void searchByRange(const Y &, const Y &,
                     std::vector<std::shared_ptr<certainData> > &) const;
};

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
class SecIndexNode_File final {
public:
  bool leaf;
  std::string path, fileName;
  std::vector<std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > > keys;
  std::vector<std::string> childrenFiles;
  StringPool &str_pool;

  explicit SecIndexNode_File(const bool _leaf, StringPool &_str_pool) : leaf(_leaf), str_pool(_str_pool) {
  }

  explicit SecIndexNode_File(const bool _leaf, std::string _path, std::string _fileName,
                             StringPool &_str_pool) : leaf(_leaf),
                                                      path(std::move(_path)), fileName(std::move(_fileName)),
                                                      str_pool(_str_pool) {
  }

  void insertNonFull(Y &, std::shared_ptr<certainData> &);

  void splitChild(int, const std::string &);

  std::vector<std::shared_ptr<certainData> > search(const Y &);

  void update(Y, std::shared_ptr<certainData> &);

  void searchByRange(const Y &, const Y &, std::vector<std::shared_ptr<certainData> > &);

  void saveToFile(const std::string &) const;

  static std::shared_ptr<SecIndexNode_File> loadFromFile(const std::string &filename, StringPool &_str_pool);

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

//======================================================================================================================

template<typename Y>
void SecIndex_File<Y>::insert(Y key, std::shared_ptr<certainData> &data) {
  std::ifstream file(source + "/root.dat", std::ios::binary);

  if (!file) {
    CreateDirectory(source.c_str(), nullptr);

    auto rootNode = std::make_shared<SecIndexNode_File<Y> >(true, source, "/root.dat", str_pool);

    rootFile = rootNode->path + rootNode->fileName;

    rootNode->insertNonFull(key, data);
  } else {
    rootFile = source + "/root.dat";

    if (SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool)->keys.size() == 2 * T - 1) {
      auto newRootNode = std::make_shared<SecIndexNode_File<Y> >(false, source, "/root.dat", str_pool);

      newRootNode->splitChild(0, source + "/root.dat");
      newRootNode->saveToFile(source + "/root.dat");
    }

    std::shared_ptr<SecIndexNode_File<Y> > node = std::move(SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool));

    node->insertNonFull(key, data);

    file.close();
  }
}

//----------------------------------------------------------------------------------------------------------------------


template<typename Y>
void SecIndex_File<Y>::remove(const Y &key, const int &id) {
  if (SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool)) {
    SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool)->remove(key, id);

    if (SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool)->keys.empty())
      deleteDirectory(StringToWString(removeFileName(source + "/root.dat", "root.dat")));
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
std::vector<std::shared_ptr<certainData> > SecIndex_File<Y>::search(const Y &key) {
  if (SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool))
    return SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool)->search(key);

  return {};
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndex_File<Y>::update(Y key, std::shared_ptr<certainData> &data) {
  if (SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool))
    return SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool)->update(key, data);
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndex_File<Y>::searchByRange(const Y &minKey, const Y &maxKey,
                                     std::vector<std::shared_ptr<certainData> > &vec) const {
  if (minKey > maxKey)
    throw BTreeError("Поменяте границы диапазона местами!\n После повторите попытку.\n");

  if (SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool)) {
    SecIndexNode_File<Y>::loadFromFile(source + "/root.dat", str_pool)->searchByRange(minKey, maxKey, vec);
  }
}

//======================================================================================================================

template<typename Y>
void SecIndexNode_File<Y>::insertNonFull(Y &key, std::shared_ptr<certainData> &data) {
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
                 std::make_shared<Data<Y, std::vector<std::shared_ptr<certainData> > > >(std::move(key), std::move(_)));

    saveToFile(path + fileName);
  } else {
    if (loadFromFile(childrenFiles[i + 1], str_pool)->keys.size() == 2 * T - 1) {
      splitChild(i + 1, childrenFiles[i + 1]);
      saveToFile(path + fileName);

      if (key > keys[i]->key)
        ++i;
    }

    loadFromFile(childrenFiles[i + 1], str_pool)->insertNonFull(key, data);
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_File<Y>::splitChild(const int i, const std::string &yFile) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 100000);


  auto y = loadFromFile(yFile, str_pool);

  if (y->fileName == "/root.dat") {
    do {
      y->fileName = "/node" + std::to_string(distrib(gen)) + ".dat";
    } while (std::ifstream(path + y->fileName, std::ios::binary)) ;
    childrenFiles.emplace(childrenFiles.begin() + i, y->path + y->fileName);
  }

  std::string z_filename;

  do {
    z_filename = "/node" + std::to_string(distrib(gen)) + ".dat";
  } while (std::ifstream(z_filename, std::ios::binary)) ;

  auto z = std::make_shared<SecIndexNode_File>(y->leaf, path, z_filename, str_pool);

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
std::vector<std::shared_ptr<certainData> > SecIndexNode_File<Y>::search(const Y &key) {
  int idx = 0;

  while (idx < keys.size() && key > keys[idx]->key)
    ++idx;

  if (idx != keys.size() && keys[idx]->key == key)
    return keys[idx]->data;

  if (!leaf && !childrenFiles[idx].empty())
    return loadFromFile(childrenFiles[idx], str_pool)->search(key);

  return {};
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_File<Y>::update(Y key, std::shared_ptr<certainData> &data) {
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

    saveToFile(path + fileName);
    return;
  }

  if (!leaf && !childrenFiles[idx].empty())
    loadFromFile(childrenFiles[idx], str_pool)->update(key, data);
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_File<Y>::searchByRange(const Y &minKey, const Y &maxKey,
                                         std::vector<std::shared_ptr<certainData> > &vec) {
  int idx = 0;

  while (idx < keys.size() && keys[idx]->key < minKey)
    ++idx;

  while (idx < keys.size() && keys[idx]->key >= minKey && keys[idx]->key <= maxKey) {
    for (int i = 0; i < keys[idx]->data.size(); ++i) {
      vec.emplace_back(keys[idx]->data[i]);
    }

    if (!leaf && childrenFiles.size() > idx && !childrenFiles[idx].empty())
      loadFromFile(childrenFiles[idx], str_pool)->searchByRange(minKey, maxKey, vec);

    ++idx;
  }

  if (!leaf && childrenFiles.size() > idx && !childrenFiles[idx].empty())
    loadFromFile(childrenFiles[idx], str_pool)->searchByRange(minKey, maxKey, vec);
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_File<Y>::remove(const Y &key, const int &id) {
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
      saveToFile(path);
    } else {
      removeFromNonLeaf(idx, id);
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
      return remove(key, id);
    }

    loadFromFile(childrenFiles[idx + navNum], str_pool)->remove(key, id);
  }
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_File<Y>::removeFromNonLeaf(const int &idx, const int &id) {
  Y key = keys[idx]->key;

  if (loadFromFile(childrenFiles[idx + 1], str_pool)->keys.size() >= T) {
    std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > replacement = getSucc(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = std::move(replacement->data);

    loadFromFile(childrenFiles[idx + 1], str_pool)->remove(replacement->key, id);
  } else if (loadFromFile(childrenFiles[idx], str_pool)->keys.size() >= T) {
    std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > replacement = getPred(idx);
    keys[idx]->key = replacement->key;
    keys[idx]->data = std::move(replacement->data);

    loadFromFile(childrenFiles[idx + 1], str_pool)->remove(replacement->key, id);
  } else {
    merge(idx);
    loadFromFile(childrenFiles[idx], str_pool)->remove(key, id);
  }

  saveToFile(path + fileName);
}

//======================================================================================================================

template<typename Y>
void SecIndexNode_File<Y>::saveToFile(const std::string &filename) const {
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

  for (auto &el: keys) {
    serializeData(file, el->key);
    serializeData(file, el->data.size());
    for (auto &el_data: el->data)
      serializeData(file, el_data);
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
std::shared_ptr<SecIndexNode_File<Y> > SecIndexNode_File<Y>::loadFromFile(
  const std::string &filename, StringPool &_str_pool) {
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

  auto node = std::make_shared<SecIndexNode_File>(leaf, _path, _fileName, _str_pool);

  int numKeys;
  file.read(reinterpret_cast<char *>(&numKeys), sizeof(numKeys));

  for (int i = 0; i < numKeys; i++) {
    Y key;
    int count;
    std::vector<std::shared_ptr<certainData> > SecIndex_File_Vec;
    std::shared_ptr<certainData> SecIndex_File;

    deserializeData(file, key);
    deserializeData(file, count);
    for (int i = 0; i < count; ++i) {
      deserializeData(file, SecIndex_File, _str_pool);
      SecIndex_File_Vec.emplace_back(SecIndex_File);
    }

    node->keys.emplace_back(
      std::make_shared<Data<Y, std::vector<std::shared_ptr<certainData> > > >(std::move(key), std::move(SecIndex_File_Vec)));
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
void SecIndexNode_File<Y>::borrowFromNext(const int &idx) {
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
void SecIndexNode_File<Y>::borrowFromPrev(const int &idx) {
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
void SecIndexNode_File<Y>::merge(const int &idx) {
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
std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > SecIndexNode_File<Y>::getPred(const int &idx) {
  auto res = loadFromFile(childrenFiles[idx], str_pool);

  while (!res->leaf)
    res = loadFromFile(res->childrenFiles.back(), str_pool);

  return res->keys.back();
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
std::shared_ptr<Data<Y, std::vector<std::shared_ptr<certainData> > > > SecIndexNode_File<Y>::getSucc(const int &idx) {
  auto res = loadFromFile(childrenFiles[idx + 1], str_pool);

  while (!res->leaf)
    res = loadFromFile(res->childrenFiles.front(), str_pool);

  return res->keys.front();
}

//----------------------------------------------------------------------------------------------------------------------

template<typename Y>
void SecIndexNode_File<Y>::fill(const int &idx) {
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

#endif //SECONDARYINDEX_FILE_H
