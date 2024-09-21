#ifndef CERTAINDATA_H
#define CERTAINDATA_H

#include <string>
#include <unordered_map>
#include <memory>
#include <utility>

struct certainData {
  int id;
  std::shared_ptr<std::string> name;

  std::unordered_map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > extraData;

  certainData(const int _id, std::shared_ptr<std::string> _name,
              const std::unordered_map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > &
              _extraData) : id(_id),
                            name(std::move(_name)),
                            extraData(_extraData) {
  }

  certainData() : id(0) {
  }
};

#endif //CERTAINDATA_H
