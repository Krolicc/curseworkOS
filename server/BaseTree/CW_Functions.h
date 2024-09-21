#ifndef CW_FUNCTIONS_H
#define CW_FUNCTIONS_H

#include "../Patterns/SingletonFlyweightPattern.h"

template<typename CommonFile>
bool checkExistance(CommonFile &tree, std::string &pullName,
                    std::string &schemaName, std::string &collectionName, StringPool &str_pull) {
  if (!pullName.empty()) {
    auto pull = tree.load(pullName);
    if (!pull)
      throw BTreeError(
        std::string("Пула с именем = \"") + pullName + "\" не существует!\nПроверьте корректность данных и попробуйте снова\n");

    if (!schemaName.empty()) {
      auto schema = pull->load(schemaName);
      if (!schema)
        throw BTreeError(
          std::string("Схемы с именем = \"") + schemaName + "\" не существует!\nПроверьте корректность данных и попробуйте снова\n");

      if (!collectionName.empty()) {
        auto collection = schema->load(collectionName, str_pull);
        if (!collection)
          throw BTreeError(
            std::string("Коллекции с именем = \"") + collectionName + "\" не существует!\nПроверьте корректность данных и попробуйте снова\n");
      }
    }
  }


  return true;
}

#endif //CW_FUNCTIONS_H
