#ifndef INC_1_H
#define INC_1_H

#include <utility>

#include <memory>

#include "../BaseTree/BTree.h"
#include "../BaseTree/certainData.h"

#include "../BaseTree/CommonWrapper.h"

#include "../Server.h"

class Command {
public:
  virtual void execute() = 0;

  virtual std::string getName() = 0;

  virtual std::shared_ptr<Command> getRevCommand() = 0;

  virtual ~Command() = default;
};

//======================================================================================================================

class Add_Pool final : public Command {
public:
  Add_Pool(std::string _poolName, CommonWrapper &_manager, StringPool &_string_pool)
    : manager(_manager), string_pool(_string_pool), poolName(std::move(_poolName)) {
  }

  void execute() override {
    std::string flag = "pull";
    manager.insert("", poolName, flag);
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override;

private:
  CommonWrapper &manager;
  StringPool &string_pool;

  std::string poolName;

  std::string name = "ADD_POOL";
};

//----------------------------------------------------------------------------------------------------------------------


class Remove_Pool final : public Command {
public:
  Remove_Pool(std::string _poolName, CommonWrapper &_manager, StringPool &_string_pool) : manager(_manager),
    string_pool(_string_pool), poolName(std::move(_poolName)) {
  }

  void execute() override {
    std::string flag = "pull";
    manager.remove("", poolName, flag);
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override {
    return std::make_unique<Add_Pool>(poolName, manager, string_pool);
  }

private:
  CommonWrapper &manager;
  StringPool &string_pool;

  std::string poolName;

  std::string name = "REMOVE_POOL";
  std::shared_ptr<Command> reverse_command = std::make_unique<Add_Pool>(poolName, manager, string_pool);
};

inline std::shared_ptr<Command> Add_Pool::getRevCommand() {
  return std::make_unique<Remove_Pool>(poolName, manager, string_pool);
}

//======================================================================================================================


class Add_Schema final : public Command {
public:
  Add_Schema(std::string _poolName, std::string _schemaName, CommonWrapper &_manager,
             StringPool &_string_pool) : manager(_manager), string_pool(_string_pool),
                                         poolName(std::move(_poolName)),
                                         schemaName(std::move(_schemaName)) {
  }

  void execute() override {
    std::string flag = "schema";
    manager.insert(poolName, schemaName, flag);
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override;

private:
  CommonWrapper &manager;
  StringPool &string_pool;

  std::string poolName, schemaName;

  std::string name = "ADD_SCHEMA";
};

//----------------------------------------------------------------------------------------------------------------------


class Remove_Schema final : public Command {
public:
  Remove_Schema(std::string _poolName, std::string _schemaName, CommonWrapper &_manager,
                StringPool &_string_pool) : manager(_manager), string_pool(_string_pool),
                                            poolName(std::move(_poolName)), schemaName(std::move(_schemaName)) {
  }

  void execute() override {
    std::string flag = "schema";
    manager.remove(poolName, schemaName, flag);
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override {
    return std::make_unique<Add_Schema>(poolName, schemaName, manager, string_pool);
  }

private:
  CommonWrapper &manager;
  StringPool &string_pool;

  std::string poolName, schemaName;

  std::string name = "REMOVE_SCHEMA";
};


inline std::shared_ptr<Command> Add_Schema::getRevCommand() {
  return std::make_unique<Remove_Schema>(poolName, schemaName, manager, string_pool);
}

//======================================================================================================================


class Add_Collection final : public Command {
public:
  Add_Collection(std::string _poolName, std::string _schemaName, std::string _collectionName,
                 CommonWrapper &_manager, StringPool &_string_pool): manager(_manager), string_pool(_string_pool),
                                                                     poolName(std::move(_poolName)),
                                                                     schemaName(std::move(_schemaName)),
                                                                     collectionName(std::move(_collectionName)) {
  }

  void execute() override {
    std::string flag = "collection";
    manager.insert(poolName + ' ' + schemaName, collectionName, flag);
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override;

private:
  CommonWrapper &manager;
  StringPool &string_pool;

  std::string poolName, schemaName, collectionName, mode;

  std::string name = "ADD_COLLECTION";
};

//----------------------------------------------------------------------------------------------------------------------


class Remove_Collection final : public Command {
public:
  Remove_Collection(std::string _poolName, std::string _schemaName, std::string _collectionName,
                    CommonWrapper &_manager, StringPool &_string_pool) : manager(_manager),
                                                                         string_pool(_string_pool),
                                                                         poolName(std::move(_poolName)),
                                                                         schemaName(std::move(_schemaName)),
                                                                         collectionName(
                                                                           std::move(_collectionName)) {
  }

  void execute() override {
    std::string flag = "collection";
    manager.remove(poolName + ' ' + schemaName, collectionName, flag);
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override {
    return std::make_unique<Add_Collection>(poolName, schemaName, collectionName, manager, string_pool);
  }

private:
  CommonWrapper &manager;
  StringPool &string_pool;

  std::string poolName, schemaName, collectionName;

  std::string name = "REMOVE_COLLECTION";
  std::shared_ptr<Command> reverse_command = std::make_unique<Add_Collection>(
    poolName, schemaName, collectionName, manager, string_pool);
};


inline std::shared_ptr<Command> Add_Collection::getRevCommand() {
  return std::make_unique<Remove_Collection>(poolName, schemaName, collectionName, manager, string_pool);
}

//======================================================================================================================


class Add_Data final : public Command {
public:
  Add_Data(std::string _poolName, std::string _schemaName, std::string _collectionName, CommonWrapper &_manager,
           StringPool &_string_pool,
           std::string _str) : manager(_manager),
                               string_pool(_string_pool),
                               poolName(std::move(_poolName)),
                               schemaName(std::move(_schemaName)),
                               collectionName(
                                 std::move(_collectionName)),
                               str(std::move(_str)) {
  }

  void execute() override {
    std::string flag = "data";
    manager.insert(poolName + ' ' + schemaName + ' ' + collectionName, str, flag);
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override;

private:
  CommonWrapper &manager;
  StringPool &string_pool;

  std::string poolName, schemaName, collectionName, str;
  std::unordered_map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > value;

  std::string name = "ADD_DATA";
  std::shared_ptr<Command> reverse_command;
};

//----------------------------------------------------------------------------------------------------------------------


class Remove_Data final : public Command {
public:
  Remove_Data(std::string _poolName, std::string _schemaName, std::string _collectionName,
              CommonWrapper &_manager, StringPool &_string_pool, std::string _key) : manager(_manager),
    string_pool(_string_pool),
    poolName(std::move(_poolName)),
    schemaName(std::move(_schemaName)),
    collectionName(std::move(_collectionName)),
    key(std::move(_key)) {
  }

  void execute() override {
    std::string flag = "data";
    _ = manager.getData(poolName + ' ' + schemaName + ' ' + collectionName, key);
    manager.remove(poolName + ' ' + schemaName + ' ' + collectionName, key, flag);
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override {
    std::ostringstream oss;

    oss << _->id << ' ' << *_->name << ' ' << _->extraData.size();

    for (const auto &[key, value]: _->extraData) {
      oss << ' ' << *key << ' ' << *value;
    }

    return std::make_unique<Add_Data>(poolName, schemaName, collectionName, manager, string_pool, oss.str());
  }

private:
  CommonWrapper &manager;
  StringPool &string_pool;

  std::string poolName, schemaName, collectionName, key;

  std::string name = "REMOVE_DATA";
  std::shared_ptr<certainData> _;
};


inline std::shared_ptr<Command> Add_Data::getRevCommand() {
  std::string key;

  std::istringstream iss(str);

  iss >> key;

  return std::make_unique<Remove_Data>(
    poolName, schemaName, collectionName, manager, string_pool, key);
}

//----------------------------------------------------------------------------------------------------------------------


class Update_Data final : public Command {
public:
  Update_Data(std::string _poolName, std::string _schemaName, std::string _collectionName,
              CommonWrapper &_manager, StringPool &_string_pool, std::string _str) : poolName(std::move(_poolName)),
    schemaName(std::move(_schemaName)),
    collectionName(std::move(_collectionName)),
    manager(_manager),
    string_pool(_string_pool),
    str(std::move(_str)) {
    std::string key;
    std::istringstream iss(str);
    iss >> key;

    _ = manager.getData(poolName + ' ' + schemaName + ' ' + collectionName, key);
  }

  void execute() override {
    manager.update(poolName + ' ' + schemaName + ' ' + collectionName, str);
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override {
    std::ostringstream oss;
    auto sooome = _;
    oss << _->id << ' ' << *_->name << ' ' << _->extraData.size() << ' ';

    for (const auto &[key, value]: _->extraData) {
      oss << *key << ' ' << *value << ' ';
    }

    return std::make_unique<Update_Data>(
      poolName, schemaName, collectionName, manager, string_pool, oss.str());
  }

private:
  CommonWrapper &manager;
  StringPool &string_pool;

  std::string poolName, schemaName, collectionName, str;

  std::string name = "UPDATE_DATA";
  std::shared_ptr<certainData> _;
};

//----------------------------------------------------------------------------------------------------------------------


class ReadRange_Data final : public Command {
public:
  ReadRange_Data(std::string _poolName, std::string _schemaName, std::string _collectionName,
                 CommonWrapper &_manager, StringPool &_string_pool,
                 std::string _type, std::string _minKey, std::string _maxKey, Server &_server) : manager(_manager),
    string_pool(_string_pool),
    poolName(std::move(_poolName)),
    schemaName(std::move(_schemaName)),
    collectionName(std::move(_collectionName)),
    type(std::move(_type)),
    minKey(std::move(_minKey)),
    maxKey(std::move(_maxKey)),
    server(_server) {
  }

  void execute() override {
    std::string res = manager.printByRange(poolName + ' ' + schemaName + ' ' + collectionName, type, minKey, maxKey).
        str();
    res = res.empty() ? "Не найдено элементов в этом диапазоне\n" : res;

    server.sendResponseAndGetResponse(res + "Для продолжения введите любую строку");
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override {
    std::shared_ptr<Command> reverse_command = nullptr;
    return reverse_command;
  }

private:
  CommonWrapper &manager;
  StringPool &string_pool;
  Server &server;
  std::string poolName, schemaName, collectionName, type, minKey, maxKey;

  std::string name = "READ_RANGE_DATA";
};

//----------------------------------------------------------------------------------------------------------------------


class ReadOne_Data final : public Command {
public:
  ReadOne_Data(std::string _poolName, std::string _schemaName, std::string _collectionName,
               CommonWrapper &_manager, StringPool &_string_pool, std::string _type,
               std::string _key, Server &_server) : manager(_manager), string_pool(_string_pool),
                                                    poolName(std::move(_poolName)),
                                                    schemaName(std::move(_schemaName)),
                                                    collectionName(std::move(_collectionName)),
                                                    key(std::move(_key)),
                                                    type(std::move(_type)),
                                                    server(_server) {
  }

  void execute() override {
    server.sendResponseAndGetResponse(
      manager.printByOne(poolName + ' ' + schemaName + ' ' + collectionName, type, key).str() +
      "Для продолжения введите любую строку");
  }

  std::string getName() override {
    return name;
  }

  std::shared_ptr<Command> getRevCommand() override {
    std::shared_ptr<Command> reverse_command = nullptr;
    return reverse_command;
  }

private:
  CommonWrapper &manager;
  StringPool &string_pool;
  Server &server;
  std::string poolName, schemaName, collectionName, key, type;

  std::string name = "READ_ONE_DATA";
};

#endif //INC_1_H
