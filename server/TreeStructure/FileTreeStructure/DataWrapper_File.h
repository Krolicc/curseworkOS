#ifndef DataWrapper_File_H
#define DataWrapper_File_H

#include "Data_File.h"
#include "SecondaryIndex_File.h"
#include "../../Patterns/SingletonFlyweightPattern.h"

using StringNode = Data<std::string, std::string>;

class DataWrapper_File : BTree<std::string, std::string, std::string> {
public:
  std::string prefix;
  std::string catalog;
  StringPool &str_pool;

  explicit DataWrapper_File(std::string _prefix, std::string _catalog,
                            StringPool &_str_pool) : prefix(std::move(_prefix)),
                                                     catalog(std::move(_catalog)), str_pool(_str_pool) {
  }

  [[nodiscard]] std::shared_ptr<RealData_File<int> > loadTreeByIdKey() const;

  [[nodiscard]] std::shared_ptr<SecIndex_File<std::string> >
  loadTreeByNameKey() const;

  void insert(std::string) override {
  }

  void insert(const std::string &, StringPool &strPull);

  void remove(const std::string &) override;

  std::ostringstream printByOne(const std::string &, const std::string &);

  std::ostringstream printByRange(const std::string &, const std::string &, const std::string &) const;

  void update(const std::string &, StringPool &strPull);

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

inline std::shared_ptr<RealData_File<int> > DataWrapper_File::loadTreeByIdKey() const {
  return std::move(
    std::make_shared<RealData_File<int> >(prefix + "/id/" + catalog, prefix + "/id/" + catalog + "/root.dat",
                                          str_pool));
}

//----------------------------------------------------------------------------------------------------------------------

inline std::shared_ptr<SecIndex_File<std::string> > DataWrapper_File::loadTreeByNameKey() const {
  return std::move(
    std::make_shared<SecIndex_File<std::string> >(prefix + "/name/" + catalog,
                                                  prefix + "/name/" + catalog + "/root.dat", str_pool));
}

//======================================================================================================================

inline void DataWrapper_File::insert(const std::string &basic_string, StringPool &strPull) {
  std::istringstream iss(basic_string);

  int id, keyCount;
  std::string name, key, value;

  iss >> id >> name >> keyCount;

  auto checkExistanceData = search(std::to_string(id), "id");

  if (!checkExistanceData.empty())
    throw BTreeError(
      std::string("Элемент с идентификатором = \"") + std::to_string(id) +
      "\" уже существует!\nИзмените идентификатор и повторите попытку\n");

  std::unordered_map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > extraData;

  while (keyCount--) {
    iss >> key >> value;
    extraData[strPull.getString(key)] = strPull.getString(value);
  }

  CreateDirectory((prefix + "/id").c_str(), nullptr);
  CreateDirectory((prefix + "/name").c_str(), nullptr);

  std::shared_ptr<certainData> data = std::make_shared<certainData>(id, strPull.getString(name), extraData);

  loadTreeByIdKey()->insert(id, data);

  loadTreeByNameKey()->insert(name, data);
}

//----------------------------------------------------------------------------------------------------------------------

inline void DataWrapper_File::remove(const std::string &idx) {
  auto el = search(idx, "id");

  if (el.empty() || !el[0])
    throw BTreeError(
      std::string("Элемент с идентификатором = \"") + idx +
      "\" не существует!\nИзмените идентификатор и повторите попытку\n");

  loadTreeByIdKey()->remove(el[0]->id);
  loadTreeByNameKey()->remove(*el[0]->name, el[0]->id);

  if (std::ifstream file(prefix + "/id/" + catalog + "/root.dat", std::ios::binary); !file)
    deleteDirectory(StringToWString(prefix + "/id"));

  if (std::ifstream file(prefix + "/name/" + catalog + "/root.dat", std::ios::binary); !file)
    deleteDirectory(StringToWString(prefix + "/name"));
}

//----------------------------------------------------------------------------------------------------------------------

inline std::ostringstream DataWrapper_File::printByOne(const std::string &key, const std::string &keyType) {
  std::ostringstream oss;

  auto vec = search(key, keyType);

  if (vec.empty()) {
    oss << "Не найдено элементов с ключом " << key << '\n';
    return std::move(oss);
  }

  for (const auto &el: vec) {
    oss << "id = \"" << el->id << "\"\n";
    oss << "name = \"" << el->name << "\"\n";
    for (const auto &[key, value]: el->extraData) {
      oss << "  " << *key << " : " << *value << "\n";
    }

    oss << '\n';
  }

  return std::move(oss);
}

//----------------------------------------------------------------------------------------------------------------------

inline std::vector<std::shared_ptr<certainData> > DataWrapper_File::search(
  const std::string &key, const std::string &keyType) {
  if (keyType == "id") {
    std::vector<std::shared_ptr<certainData> > _;
    if (const auto res = loadTreeByIdKey()->search(stoi(key))) {
      _.emplace_back(res->data);
      return std::move(_);
    }

    return {};
  }

  if (keyType == "name") {
    if (const auto res = loadTreeByNameKey()->search(key); !res.empty())
      return res;

    return {};
  }

  std::cout << "Incorrect type!\n";
  return {};
}

//----------------------------------------------------------------------------------------------------------------------

inline void DataWrapper_File::searchByRange(
  const std::string &keyType, const std::string &minKey, const std::string &maxKey,
  std::vector<std::shared_ptr<certainData> > &vec) const {
  if (keyType == "id")
    loadTreeByIdKey()->searchByRange(stoi(minKey), stoi(maxKey), vec);
  else if (keyType == "name")
    loadTreeByNameKey()->searchByRange(minKey, maxKey, vec);
  else
    std::cout << "Incorrect type!\n";
}

//----------------------------------------------------------------------------------------------------------------------

inline std::ostringstream DataWrapper_File::printByRange(const std::string &keyType, const std::string &minKey,
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

inline void DataWrapper_File::update(const std::string &basic_string, StringPool &strPull) {
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

  loadTreeByIdKey()->update(id, data);
  loadTreeByNameKey()->update(name, data);
}

#endif //DataWrapper_File_H
