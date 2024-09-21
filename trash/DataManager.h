#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <iostream>
#include <unordered_map>
#include <utility>

#include "tree.h"

// ����� ��� ���������� ����������� ������ � ������� ������, ������ ������
class DataManager {
private:
  // ��� ����������� ��� � ��� (��������� ��� � ����� ��� ������ in memory cache)
  std::string file;
  std::unordered_map<std::shared_ptr<std::string>,
    std::unordered_map<std::shared_ptr<std::string>,
      std::unordered_map<std::shared_ptr<std::string>,
        SecondaryIndex *> > > pools;
  bool workingMode_isFile = false;
  std::string storageName;


  // ��������� �����, ����������� � �����
  void saveToFile(const std::string &name) const {
    std::ofstream file("../server/storageMemorySystem/" + name + ".txt");

    file << name << ' ' << pools.size() << '\n';

    for (const auto &[poolsName, schema]: pools) {
      file << "  " << *poolsName << ' ' << schema.size() << '\n';

      for (const auto &[schemaName, collection]: schema) {
        file << "    " << *schemaName << ' ' << collection.size() << '\n';

        for (const auto &[collectionName, data]: collection) {
          file << "      " << *collectionName << ' ' << data->getData().size() << '\n';

          for (const auto &[dataName, extraParams]: data->getData()) {
            file << "        " << *extraParams->name << ' ' << extraParams->extraData.size() << '\n';
            file << "          " << "id = " << extraParams->id << '\n';

            for (const auto &[key, value]: extraParams->extraData)
              file << "          " << *key << " : " << *value << '\n';
          }
        }
      }
      file << '\n';
    }
  }

  // ��������� �����, ������������ �� �����
  void loadFromFile(const std::string &name, StringPool &string_pool) {
    std::ifstream file("../server/storageMemorySystem/" + name + ".txt");
    if (!file) return;

    std::string key, value, dataName, collectionName, schemaName, poolName, poolsName, _idWord;
    char _separator;
    int dataSize, collectionSize, schemaSize, poolSize, poolsSize, id;

    file >> poolsName >> poolsSize;

    for (int _q = 1; _q <= poolsSize; ++_q) {
      file >> poolName >> poolSize;

      std::unordered_map<std::shared_ptr<std::string>,
        std::unordered_map<std::shared_ptr<std::string>,
          SecondaryIndex *> > poolsData;
      for (int _i = 1; _i <= poolSize; ++_i) {
        file >> schemaName >> schemaSize;

        std::unordered_map<std::shared_ptr<std::string>,
          SecondaryIndex *> schemaData;
        for (int _j = 1; _j <= schemaSize; ++_j) {
          file >> collectionName >> collectionSize;

          auto collectionData = new SecondaryIndex;
          for (int _k = 1; _k <= collectionSize; ++_k) {
            file >> dataName >> dataSize;

            file >> _idWord >> _separator >> id;
            std::map<std::shared_ptr<std::string>, std::shared_ptr<std::string>> Data;
            for (int _l = 1; _l <= dataSize; ++_l) {
              file >> key >> _separator >> value;
              Data[string_pool.getString(key)] = string_pool.getString(value);
            }
            collectionData->add(string_pool, dataName, Data, id);
          }
          schemaData[string_pool.getString(collectionName)] = collectionData;
        }
        poolsData[string_pool.getString(schemaName)] = schemaData;
      }
      pools[string_pool.getString(poolName)] = poolsData;
    }
  }

public:
  explicit DataManager(StringPool &string_pool, const bool _workingMode_isFile, std::string _storageName = "storageFileSystem") : workingMode_isFile(
      _workingMode_isFile),
    storageName(std::move(_storageName)) {
    if (!workingMode_isFile) loadFromFile(storageName, string_pool);
  }

  // ����������, ��������� ��������� ��� ������ ������� ������
  ~DataManager() {
    if (!workingMode_isFile) saveToFile(storageName);

    for (auto &[_, schemas]: pools) {
      for (auto &[_, collection]: schemas) {
        for (auto &[_, container]: collection) {
          delete container;
        }
      }
    }
  }

  //--------------------------------------------------------------------------------------------------------------------

  // ����� �������� ������������� ���� ���� ������
  bool pool_isExist(const std::string &poolName, StringPool &string_pool) const {
    if (pools.contains(string_pool.getString(poolName)))
      return true;

    std::cerr << "���� � ����� ������ �� ���������!" << std::endl;
    return false;
  }

  //--------------------------------------------------------------------------------------------------------------------

  // ����� �������� ������������� ����� ������
  bool schema_isExist(const std::string &poolName, const std::string &schemaName, StringPool &string_pool) {
    if (pool_isExist(poolName, string_pool)) {
      if (pools[string_pool.getString(poolName)].contains(string_pool.getString(schemaName)))
        return true;

      std::cerr << "����� ������ � ����� ������ �� ���������!" << std::endl;
    }

    return false;
  }

  //--------------------------------------------------------------------------------------------------------------------

  // ����� �������� ������������� ��������� ������
  bool collection_isExist(const std::string &poolName, const std::string &schemaName,
                          const std::string &collectionName, StringPool &string_pool) {
    if (schema_isExist(poolName, schemaName, string_pool)) {
      if (pools[string_pool.getString(poolName)][string_pool.getString(schemaName)].contains(
        string_pool.getString(collectionName)))
        return true;

      std::cerr << "��������� ������ � ����� ������ �� ���������!" << std::endl;
    }
    return false;
  }

  //====================================================================================================================

  // ����� ���������� ���� ���� ������
  void addPool(const std::string &poolName, StringPool &string_pool) {
    // ������� ���
    pools[string_pool.getString(poolName)] = std::unordered_map<std::shared_ptr<std::string>,
      std::unordered_map<std::shared_ptr<std::string>,
        SecondaryIndex *> >();
  }

  //--------------------------------------------------------------------------------------------------------------------

  // ����� �������� ���� ���� ������
  void removePool(const std::string &poolName, StringPool &string_pool) {
    if (!pool_isExist(poolName, string_pool))
      return;

    // ������� ������ ��� ������ ����
    for (auto &[schemaName, schema]: pools[string_pool.getString(poolName)]) {
      for (auto &[_, collection]: schema) {
        delete collection;
      }

      pools[string_pool.getString(poolName)].erase(schemaName);
    }

    // ����� ������� ��� ����
    pools.erase(string_pool.getString(poolName));
  }

  //====================================================================================================================

  // ����� ���������� ����� ������
  void addSchema(const std::string &poolName, const std::string &schemaName, StringPool &string_pool) {
    if (!pool_isExist(poolName, string_pool))
      return;

    pools[string_pool.getString(poolName)][string_pool.getString(schemaName)] = std::unordered_map<std::shared_ptr<
      std::string>, SecondaryIndex *>();
  }

  //--------------------------------------------------------------------------------------------------------------------

  // �������� ����� ������
  void removeSchema(const std::string &poolName, const std::string &schemaName, StringPool &string_pool) {
    if (!schema_isExist(poolName, schemaName, string_pool))
      return;

    // ������� ������� ������ ����� ������
    for (auto &[_, collection]: pools[string_pool.getString(poolName)][string_pool.getString(schemaName)]) {
      delete collection;
    }

    // ����� ������� ������ � ���
    pools[string_pool.getString(poolName)].erase(string_pool.getString(schemaName));
  }

  //====================================================================================================================

  // ���������� ��������� ������
  void addCollection(const std::string &poolName, const std::string &schemaName, const std::string &collectionName,
                     SecondaryIndex *container, StringPool &string_pool) {
    if (!schema_isExist(poolName, schemaName, string_pool))
      return;

    pools[string_pool.getString(poolName)][string_pool.getString(schemaName)][string_pool.getString(collectionName)] =
        container;
  }

  //--------------------------------------------------------------------------------------------------------------------

  // �������� ��������� ������
  void removeCollection(const std::string &poolName, const std::string &schemaName, const std::string &collectionName,
                        StringPool &string_pool) {
    if (!collection_isExist(poolName, schemaName, collectionName, string_pool))
      return;

    // ������� ������� ���� ���������
    delete pools[string_pool.getString(poolName)][string_pool.getString(schemaName)][string_pool.getString(
      collectionName)];
    // ����� ������ � ���
    pools[string_pool.getString(poolName)][string_pool.getString(schemaName)].erase(
      string_pool.getString(collectionName));
  }

  //--------------------------------------------------------------------------------------------------------------------

  // ��������� ��������� ������
  SecondaryIndex *getCollection(const std::string &poolName, const std::string &schemaName,
                                const std::string &collectionName, StringPool &string_pool) {
    if (!collection_isExist(poolName, schemaName, collectionName, string_pool))
      return nullptr;

    return pools[string_pool.getString(poolName)][string_pool.getString(schemaName)][string_pool.getString(
      collectionName)];
  }

  //==================================================================================================================

  // ���������� ������ � ��������� ������
  void addData(const std::string &poolName, const std::string &schemaName, const std::string &collectionName,
               StringPool &string_pool,
               const std::string &key, const std::map<std::shared_ptr<std::string>, std::shared_ptr<std::string>> &value) {
    if (!collection_isExist(poolName, schemaName, collectionName, string_pool))
      return;

    // ����� ������ � ���
    const auto &collection = pools[string_pool.getString(poolName)][string_pool.getString(schemaName)][string_pool.
      getString(
        collectionName)];
    collection->add(string_pool, key, value);
  }

  //--------------------------------------------------------------------------------------------------------------------

  void removeData(const std::string &poolName, const std::string &schemaName, const std::string &collectionName,
                  StringPool &string_pool,
                  const std::string &key) {
    if (!collection_isExist(poolName, schemaName, collectionName, string_pool))
      return;

    // ����� ������ � ���
    pools[string_pool.getString(poolName)][string_pool.getString(schemaName)][string_pool.getString(
      collectionName)]->remove(stoi(key));
  }

  //--------------------------------------------------------------------------------------------------------------------

  void updateData(const std::string &poolName, const std::string &schemaName, const std::string &collectionName,
                  StringPool &string_pool,
                  const std::string &key, const std::map<std::shared_ptr<std::string>, std::shared_ptr<std::string>> &value) {
    if (!collection_isExist(poolName, schemaName, collectionName, string_pool))
      return;

    // ����� ������ � ���
    pools[string_pool.getString(poolName)][string_pool.getString(schemaName)][string_pool.getString(
      collectionName)]->update(string_pool, key, value);
  }

  //--------------------------------------------------------------------------------------------------------------------

  void readOneData(const std::string &poolName, const std::string &schemaName, const std::string &collectionName,
                   StringPool &string_pool,
                   const std::string &type, const std::string &key) {
    if (!collection_isExist(poolName, schemaName, collectionName, string_pool))
      return;

    // ����� ������ � ���
    const std::vector<std::shared_ptr<Data> > &res = pools[string_pool.getString(poolName)][string_pool.
          getString(schemaName)][string_pool.getString(
          collectionName)]->
        read(type, key);

    if (!res.empty()) {
      for (const auto &data: res) {
        std::cout << data->name << "#" << data->id << ":" << std::endl;

        for (const auto &[key, value]: data->extraData)
          std::cout << "    " << key << " : " << value << std::endl;
      }
    }
  }

  //--------------------------------------------------------------------------------------------------------------------

  void readRangeData(const std::string &poolName, const std::string &schemaName, const std::string &collectionName,
                     StringPool &string_pool,
                     const std::string &type, const std::string &minKey, const std::string &maxKey) {
    if (!collection_isExist(poolName, schemaName, collectionName, string_pool))
      return;

    // ����� ������ � ���
    const std::vector<std::shared_ptr<Data> > &res =
        pools[string_pool.getString(poolName)][string_pool.getString(schemaName)][string_pool.getString(
          collectionName)]->readByRange(type, minKey, maxKey);

    if (!res.empty()) {
      for (const auto &data: res) {
        std::cout << data->name << "#" << data->id << ":" << std::endl;

        for (const auto &[key, value]: data->extraData)
          std::cout << "    " << key << " : " << value << std::endl;
      }
    }
  }
};

#endif //DATAMANAGER_H
