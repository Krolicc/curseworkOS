#ifndef TREE_H
#define TREE_H

// ��� - ����� �������� �������
// ��� - ��������������� ����������� ���������


#include <string>
#include <map>

#include <utility>

#include <fstream>

class Data {
public:
  int id;
  std::shared_ptr<std::string> name;
  std::map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > extraData;

  Data(const int _id, std::shared_ptr<std::string> _name,
       std::map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > _extraData) : id(_id),
    name(std::move(_name)),
    extraData(std::move(_extraData)) {
  }
};

//======================================================================================================================

// �����, ����������� ������ � ������
class SecondaryIndex final {
private:
  // ������ ��� �������� ���������� ��������
  int elCount;
  std::unordered_map<int, std::shared_ptr<Data> > idIndex;
  std::unordered_map<std::shared_ptr<std::string>, std::vector<std::shared_ptr<Data> > > nameIndex;

public:
  // �������� ���������� in memory cache
  SecondaryIndex() : elCount(0) {
  }

  void add(StringPool &string_pool, const std::string &key,
           const std::map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > &value, int id = -1) {
    if (id == -1) id = elCount;
    else elCount = id;

    auto data = std::make_shared<Data>(id, string_pool.getString(key), value);
    idIndex[data->id] = data;
    nameIndex[data->name].push_back(data);
    ++elCount;
  }

  // �������� ������ in memory cache
  [[nodiscard]] std::vector<std::shared_ptr<Data> >
  read(const std::string &type, const std::string &key) const {
    if (type == "name") {
      auto it = nameIndex.find(std::make_shared<std::string>(key));
      if (it != nameIndex.end()) {
        return it->second;
      }
    } else if (type == "id") {
      const int id = std::stoi(key); // �������������� ������ � int
      if (const auto it = idIndex.find(id); it != idIndex.end()) {
        std::vector<std::shared_ptr<Data> > res;
        res.push_back(it->second);
        return res;
      }
    }

    return {};
  }

  std::vector<std::shared_ptr<Data> > readByRange(const std::string &type, const std::string &minName,
                                                  const std::string &maxName) {
    if (type == "name")
      return searchByNameRange(minName, maxName);

    if (type == "id") {
      const int minId = std::stoi(minName);
      const int maxId = std::stoi(maxName);
      return searchByIdRange(minId, maxId);
    }

    return {};
  }

  // �������� ������ ��������� in memory cache
  std::vector<std::shared_ptr<Data> > searchByIdRange(const int minId, const int maxId) {
    std::vector<std::shared_ptr<Data> > res;
    for (const auto &pair: idIndex) {
      if (pair.first >= minId && pair.first <= maxId)
        res.push_back(pair.second);
    }
    return res;
  }

  // ����� �������� �� ��������� ����
  std::vector<std::shared_ptr<Data> > searchByNameRange(const std::string &minName, const std::string &maxName) {
    std::vector<std::shared_ptr<Data> > res;
    for (const auto &pair: nameIndex) {
      if (pair.first >= std::make_shared<std::string>(minName) && pair.first <= std::make_shared<std::string>(maxName)) {
        for (const auto &obj: pair.second)
          res.push_back(obj);
      }
    }

    return res;
  }

  // �������� ���������� in memory cache
  void update(StringPool &string_pool, const std::string &key, const std::map<std::shared_ptr<std::string>, std::shared_ptr<std::string>> &newValue) {
    if (idIndex.contains(stoi(key))) {
      read("id", key)[0]->extraData = newValue;
    }
  }

  // �������� ��������
  void remove(int id) {
    auto it = idIndex.find(id);
    if (it != idIndex.end()) {
      auto obj = it->second; // �������� ������

      // ������� ������ �� ������� �� �����
      auto nameIt = nameIndex.find(obj->name);
      if (nameIt != nameIndex.end()) {
        auto &objectList = nameIt->second;
        auto objIt = std::remove_if(objectList.begin(), objectList.end(),
                                    [id](const std::shared_ptr<Data> &o) {
                                      return o->id == id;
                                    });

        if (objIt != objectList.end()) {
          objectList.erase(objIt, objectList.end());
          // ���� ������ ����, ������� ���� �� nameIndex
          if (objectList.empty()) {
            nameIndex.erase(nameIt);
          }
        }
      }

      // ������� ������ �� ������� �� ��������������
      idIndex.erase(it);
    }
  }

  // ����� ��������� ������
  std::unordered_map<int, std::shared_ptr<Data> > getData() {
    return idIndex;
  }
};

#endif //TREE_H
