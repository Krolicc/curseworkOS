#ifndef SINGLETONFLYWEIGHTPATTERN_H
#define SINGLETONFLYWEIGHTPATTERN_H

#include <iostream>
#include <unordered_map>
#include <memory>
#include <string>

// Класс приспособленец
class StringPool {
public:
  // Статический метод возвращаюсщий пул строк
  static StringPool &getInstance() {
    static StringPool instance;
    return instance;
  }

  // Метод для получения/создания строк
  std::shared_ptr<std::string> getString(const std::string &str) {
    auto it = pool.find(str);

    if (it != pool.end())
      return it->second;

    auto newStr = std::make_shared<std::string>(str);
    pool[str] = newStr;
    return newStr;
  }

  // Невозможность получить копию

  StringPool(const StringPool &) = delete;

  StringPool &operator=(const StringPool &) = delete;

private:
  // Приватный конструктор Singleton
  StringPool() = default;

  // Хранилище уникальных строк
  std::unordered_map<std::string, std::shared_ptr<std::string> > pool;
};

#endif //SINGLETONFLYWEIGHTPATTERN_H
