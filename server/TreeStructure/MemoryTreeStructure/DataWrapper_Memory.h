#ifndef DataWrapper_Memory_H
#define DataWrapper_Memory_H

#include "../../ExtraFunctions.h"

#include "Data_Memory.h"
#include "SecondaryIndex_Memory.h"

#include "../../BaseTree/CW_Functions.h"

#include "../../Patterns/SingletonFlyweightPattern.h"

#include "../../ErrorHandler/ErrorThrower.h"

class DataWrapper_Memory : BTree<std::string, std::string, std::string> {
public:
  std::shared_ptr<RealData_Memory<int> > RealDataById_Memory = std::make_shared<RealData_Memory<int> >();
  std::shared_ptr<SecIndex_Memory<std::string> > SecondaryIndex_Memory = std::make_shared<SecIndex_Memory<
    std::string> >();

  explicit DataWrapper_Memory() = default;

  void insert(std::string) override {
  }

  void insert(const std::string &, StringPool &);

  void remove(const std::string &) override;

  std::ostringstream printByOne(const std::string &, const std::string &);

  std::ostringstream printByRange(const std::string &, const std::string &, const std::string &) const;

  void update(const std::string &, StringPool &);

  void saveToFile(std::ofstream &);

  void loadFromFile(std::ifstream &, StringPool &strPool);

  int getElemsCount();

  std::vector<std::shared_ptr<certainData> > search(const std::string &, const std::string &);

private:
  void searchByRange(const std::string &, const std::string &,
                     const std::string &,
                     std::vector<std::shared_ptr<certainData> > &) const;

  std::shared_ptr<Data<std::string, std::string> > search(const std::string &) override {
    return nullptr;
  }
};

//======================================================================================================================

inline void DataWrapper_Memory::saveToFile(std::ofstream &file) {
  if (RealDataById_Memory->root) {
    RealDataById_Memory->saveToFile(file);
  }
}

//----------------------------------------------------------------------------------------------------------------------

inline void DataWrapper_Memory::loadFromFile(std::ifstream &file, StringPool &strPull) {
  int elemsCount, id, extraDataCount;
  std::string name, key, value, separator;

  file >> elemsCount;

  for (int i = 0; i < elemsCount; ++i) {
    file >> id >> name >> extraDataCount;
    std::unordered_map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > extraData;
    for (int j = 0; j < extraDataCount; ++j) {
      file >> key >> separator >> value;
      extraData[strPull.getString(key)] = strPull.getString(value);
    }

    auto data = std::make_shared<certainData>(id, strPull.getString(name), extraData);

    RealDataById_Memory->insert(id, data);
    SecondaryIndex_Memory->insert(name, data);
  }
}

//======================================================================================================================

inline void DataWrapper_Memory::insert(const std::string &basic_string, StringPool &strPull) {
  std::istringstream iss(basic_string);

  int id, keyCount;
  std::string name, key, value;

  iss >> id >> name >> keyCount;

  auto checkExistanceData = RealDataById_Memory->search(id);

  if (checkExistanceData)
    throw BTreeError(
      std::string("Элемент с идентификатором = \"") + std::to_string(id) +
      "\" уже существует!\nИзмените идентификатор и повторите попытку\n");

  std::unordered_map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > extraData;

  while (keyCount--) {
    iss >> key >> value;
    extraData[strPull.getString(key)] = strPull.getString(value);
  }

  auto data = std::make_shared<certainData>(id, strPull.getString(name), extraData);

  RealDataById_Memory->insert(id, data);
  SecondaryIndex_Memory->insert(std::move(name), data);
}

//----------------------------------------------------------------------------------------------------------------------

inline int DataWrapper_Memory::getElemsCount() {
  return RealDataById_Memory->elemsCount;
}

//----------------------------------------------------------------------------------------------------------------------

inline void DataWrapper_Memory::update(const std::string &basic_string, StringPool &strPull) {
  std::istringstream iss(basic_string);

  int id, keyCount;
  std::string name, key, value;

  iss >> id >> name >> keyCount;

  std::unordered_map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > extraData;

  while (keyCount--) {
    iss >> key >> value;
    extraData[strPull.getString(key)] = strPull.getString(value);
  }

  auto data = std::make_shared<certainData>(id, strPull.getString(name), extraData);

  RealDataById_Memory->update(id, data);
  SecondaryIndex_Memory->update(name, data);
}

//----------------------------------------------------------------------------------------------------------------------


inline void DataWrapper_Memory::remove(const std::string &idx) {
  auto el = search(idx, "id");

  if (el.empty() || !el[0])
    throw BTreeError(
      std::string("Элемент с идентификатором = \"") + idx +
      "\" не существует!\nИзмените идентификатор и повторите попытку\n");

  RealDataById_Memory->remove(el[0]->id);
  SecondaryIndex_Memory->remove(*el[0]->name, el[0]->id);
}

//----------------------------------------------------------------------------------------------------------------------

inline std::ostringstream DataWrapper_Memory::printByOne(const std::string &key, const std::string &keyType) {
  std::ostringstream oss;

  auto vec = search(key, keyType);

  if (vec.empty()) {
    oss << "Не найдено элементов с ключом " << key << '\n';
    return std::move(oss);
  }

  for (const auto &el: vec) {
    oss << "id = \"" << el->id << "\"\n";
    oss << "name = \"" << *el->name << "\"\n";
    for (const auto &[key, value]: el->extraData) {
      oss << "  " << *key << " : " << *value << "\n";
    }

    oss << '\n';
  }

  return std::move(oss);
}

//----------------------------------------------------------------------------------------------------------------------

inline std::vector<std::shared_ptr<certainData> > DataWrapper_Memory::search(
  const std::string &key, const std::string &keyType) {
  if (keyType == "id" && RealDataById_Memory) {
    std::vector<std::shared_ptr<certainData> > _;
    if (const auto res = RealDataById_Memory->search(stoi(key))) {
      _.emplace_back(res->data);
      return std::move(_);
    }

    return {};
  }

  if (keyType == "name" && SecondaryIndex_Memory) {
    if (auto res = SecondaryIndex_Memory->search(key); !res.empty())
      return res;

    return {};
  }

  std::cout << "Incorrect type!\n";
  return {};
}

//----------------------------------------------------------------------------------------------------------------------

inline void DataWrapper_Memory::searchByRange(
  const std::string &keyType, const std::string &minKey, const std::string &maxKey,
  std::vector<std::shared_ptr<certainData> > &vec) const {
  if (keyType == "id" && RealDataById_Memory)
    RealDataById_Memory->searchByRange(stoi(minKey), stoi(maxKey), vec);
  else if (keyType == "name" && SecondaryIndex_Memory)
    SecondaryIndex_Memory->searchByRange(minKey, maxKey, vec);
  else
    std::cout << "Incorrect type!\n";
}

//----------------------------------------------------------------------------------------------------------------------

inline std::ostringstream DataWrapper_Memory::printByRange(const std::string &keyType, const std::string &minKey,
                                                           const std::string &maxKey) const {
  std::ostringstream oss;
  std::vector<std::shared_ptr<certainData> > vec;
  searchByRange(keyType, minKey, maxKey, vec);

  for (const auto &el: vec) {
    oss << "id = \"" << el->id << "\"\n";
    oss << "name = \"" << *el->name << "\"\n";
    for (const auto &[key, value]: el->extraData) {
      oss << "  " << *key << " : " << *value << "\n";
    }

    oss << '\n';
  }

  return std::move(oss);
}

#endif //DataWrapper_Memory_H
