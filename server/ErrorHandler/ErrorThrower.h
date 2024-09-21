#ifndef ERROR_CREATOR_H
#define ERROR_CREATOR_H

#include <string>
#include <stdexcept>

class BTreeError : public std::runtime_error {
public:
  explicit BTreeError(const std::string& message) : std::runtime_error(message) {}
};

#endif //ERROR_CREATOR_H
