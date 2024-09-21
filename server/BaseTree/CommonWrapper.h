#ifndef COMMONWRAPPER_H
#define COMMONWRAPPER_H

#include "certainData.h"

#include "../TreeStructure/FileTreeStructure/DataWrapper_File.h"
#include "../TreeStructure/FileTreeStructure/CollectionsSchema_File.h"
#include "../TreeStructure/FileTreeStructure/SchemaPool_File.h"
#include "../TreeStructure/FileTreeStructure/CommonPool_File.h"

#include "../TreeStructure/MemoryTreeStructure/DataWrapper_Memory.h"
#include "../TreeStructure/MemoryTreeStructure/CollectionsSchema_Memory.h"
#include "../TreeStructure/MemoryTreeStructure/SchemaPool_Memory.h"
#include "../TreeStructure/MemoryTreeStructure/CommonPool_Memory.h"

#include "../Patterns/SingletonFlyweightPattern.h"

#include "../Server.h"

#include "CW_Functions.h"

#include <fstream>
#include <iostream>

class CommonWrapper final {
  bool isMemoryTree;
  bool loadingAndSaving;
  std::string fileName;
  StringPool &str_pool;
  Server &server;

  CommonPool_File FileTree;
  CommonPool_Memory MemoryTree;

public:
  ~CommonWrapper() {
    if (loadingAndSaving && isMemoryTree) {
      std::string _fileName = "../server/Storage/storageMemorySystem/" + fileName + ".txt";

      std::ofstream file(_fileName);

      auto _ = MemoryTree;
      if (file) {
        MemoryTree.saveToFile(file);

        file.close();
        save_fileHash(_fileName);
      }
    }
  }

  CommonWrapper(const bool _isMemoryTree, std::string _fileName, StringPool &_str_pool,
                Server &_server, const bool _loadingAndSaving = false) : isMemoryTree(_isMemoryTree),
                                                                         loadingAndSaving(_loadingAndSaving),
                                                                         fileName(std::move(_fileName)),
                                                                         str_pool(_str_pool),
                                                                         server(_server) {
    if (loadingAndSaving && isMemoryTree) {
      std::string cur_fileName = "../server/Storage/storageMemorySystem/" + fileName + ".txt";
      std::ifstream file(cur_fileName);

      if (file.is_open()) {
        if (!check_fileHash(cur_fileName)) {
          server.sendResponseAndGetResponse(
            "‘айл не был загружен по причине его изменени€ с прошлого раза\n¬ведите любую строку дл€ продолжени€\n");
          return;
        }

        if (!isFileEmpty(cur_fileName))
          MemoryTree.loadFromFile(file, str_pool);

        file.close();
      }
    } else if (!isMemoryTree)
      FileTree.setSource("../server/Storage/storageFileSystem/" + fileName);
  }

  void insert(const std::string &path, std::string &str, std::string &flag) {
    std::istringstream iss(path);
    std::string pull, schema, collection;

    if (flag == "pull") {
      if (isMemoryTree)
        MemoryTree.insert(str);
      else
        FileTree.insert(str);
    } else if (flag == "schema") {
      iss >> pull;

      if (isMemoryTree) {
        if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
          MemoryTree.load(pull)->insert(str);
      } else {
        if (checkExistance(FileTree, pull, schema, collection, str_pool))
          FileTree.load(pull)->insert(str);
      }
    } else if (flag == "collection") {
      iss >> pull >> schema;

      if (isMemoryTree) {
        if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
          MemoryTree.load(pull)->load(schema)->insert(str);
      } else {
        if (checkExistance(FileTree, pull, schema, collection, str_pool))
          FileTree.load(pull)->load(schema)->insert(str);
      }
    } else if (flag == "data") {
      iss >> pull >> schema >> collection;

      if (isMemoryTree) {
        if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
          MemoryTree.load(pull)->load(schema)->load(collection, str_pool)->insert(str, str_pool);
      } else if (checkExistance(FileTree, pull, schema, collection, str_pool))
        FileTree.load(pull)->load(schema)->load(collection, str_pool)->insert(str, str_pool);
    }
  }

  void remove(const std::string &path, std::string &ind, std::string &flag) {
    std::istringstream iss(path);
    std::string pull, schema, collection;

    if (flag == "pull") {
      if (isMemoryTree)
        MemoryTree.remove(ind);
      else
        FileTree.remove(ind);
    } else if (flag == "schema") {
      iss >> pull;

      if (isMemoryTree) {
        if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
          MemoryTree.load(pull)->remove(ind);
      } else {
        if (checkExistance(FileTree, pull, schema, collection, str_pool))
          FileTree.load(pull)->remove(ind);
      }
    } else if (flag == "collection") {
      iss >> pull >> schema;

      if (isMemoryTree) {
        if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
          MemoryTree.load(pull)->load(schema)->remove(ind);
      } else {
        if (checkExistance(FileTree, pull, schema, collection, str_pool))
          FileTree.load(pull)->load(schema)->remove(ind);
      }
    } else if (flag == "data") {
      iss >> pull >> schema >> collection;

      if (isMemoryTree) {
        if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
          MemoryTree.load(pull)->load(schema)->load(collection, str_pool)->remove(ind);
      } else if (checkExistance(FileTree, pull, schema, collection, str_pool))
        FileTree.load(pull)->load(schema)->load(collection, str_pool)->remove(ind);
    }
  }

  void update(const std::string &path, std::string &str) {
    std::istringstream iss(path);
    std::string pull, schema, collection;

    iss >> pull >> schema >> collection;

    if (isMemoryTree) {
      if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
        MemoryTree.load(pull)->load(schema)->load(collection, str_pool)->update(str, str_pool);
    } else if (checkExistance(FileTree, pull, schema, collection, str_pool))
      FileTree.load(pull)->load(schema)->load(collection, str_pool)->update(str, str_pool);
  }

  std::ostringstream printByOne(const std::string &path, const std::string &type, const std::string &key) {
    std::istringstream iss(path);
    std::string pull, schema, collection;

    iss >> pull >> schema >> collection;

    if (isMemoryTree) {
      if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
        return MemoryTree.load(pull)->load(schema)->load(collection, str_pool)->printByOne(key, type);
    } else if (checkExistance(FileTree, pull, schema, collection, str_pool))
      return FileTree.load(pull)->load(schema)->load(collection, str_pool)->printByOne(key, type);
  }

  std::ostringstream printByRange(const std::string &path, const std::string &type, const std::string &minKey,
                                  const std::string &maxKey) {
    std::istringstream iss(path);
    std::string pull, schema, collection;

    iss >> pull >> schema >> collection;

    if (isMemoryTree) {
      if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
        return std::move(
          MemoryTree.load(pull)->load(schema)->load(collection, str_pool)->printByRange(type, minKey, maxKey));
    } else if (checkExistance(FileTree, pull, schema, collection, str_pool))
      return std::move(
        FileTree.load(pull)->load(schema)->load(collection, str_pool)->printByRange(type, minKey, maxKey));
  }

  std::shared_ptr<certainData> getData(const std::string &path, const std::string &key) {
    std::istringstream iss(path);
    std::string pull, schema, collection;

    iss >> pull >> schema >> collection;

    if (isMemoryTree)
      if (checkExistance(MemoryTree, pull, schema, collection, str_pool))
        return std::move(MemoryTree.load(pull)->load(schema)->load(collection, str_pool)->search(key, "id"))[0];

    if (checkExistance(FileTree, pull, schema, collection, str_pool))
      return std::move(FileTree.load(pull)->load(schema)->load(collection, str_pool)->search(key, "id"))[0];

    return nullptr;
  }
};

#endif //COMMONWRAPPER_H
