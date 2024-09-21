#ifndef EXTRAFUNCTIONS_H
#define EXTRAFUNCTIONS_H

#include <iostream>
#include <sstream>
#include <fstream>

#include <regex>

#include <algorithm>

#include <ctime>
#include <chrono>

#include <string>
#include <array>
#include <vector>

#include <memory>
#include <codecvt>

#include <windows.h>
#include <locale>

#include "ExtraFunctions.h"
#include "BaseTree/certainData.h"
#include "BaseTree/BTree.h"

#include "Patterns/SingletonFlyweightPattern.h"

#include "ErrorHandler/ErrorThrower.h"

inline void serializeData(std::ofstream &file, const std::string &ptr) {
  const int lengthKey = static_cast<int>(ptr.size());
  file.write(reinterpret_cast<const char *>(&lengthKey), sizeof(lengthKey));
  file.write(ptr.c_str(), lengthKey);
}

//----------------------------------------------------------------------------------------------------------------------

inline void serializeData(std::ofstream &file, const int &ptr) {
  file.write(reinterpret_cast<const char *>(&ptr), sizeof(int));
}

//----------------------------------------------------------------------------------------------------------------------

inline void serializeData(std::ofstream &file, std::shared_ptr<certainData> &ptr) {
  file.write(reinterpret_cast<const char *>(&ptr->id), sizeof(int));

  int keyLength = static_cast<int>(ptr->name->size());
  file.write(reinterpret_cast<const char *>(&keyLength), sizeof(keyLength));
  file.write(ptr->name->data(), keyLength);

  int extraDataLenght = static_cast<int>(ptr->extraData.size());
  file.write(reinterpret_cast<const char *>(&extraDataLenght), sizeof(extraDataLenght));

  for (const auto &[key, value]: ptr->extraData) {
    int wordLength = static_cast<int>(key->size());
    file.write(reinterpret_cast<const char *>(&wordLength), sizeof(wordLength));
    file.write(key->data(), wordLength);

    wordLength = static_cast<int>(value->size());
    file.write(reinterpret_cast<const char *>(&wordLength), sizeof(wordLength));
    file.write(value->data(), wordLength);
  }
}

//======================================================================================================================

inline void deserializeData(std::ifstream &file, std::string &ptr) {
  int lengthKey;
  file.read(reinterpret_cast<char *>(&lengthKey), sizeof(lengthKey));

  char *buffer = new char[lengthKey + 1];
  file.read(buffer, lengthKey);
  buffer[lengthKey] = '\0';

  ptr.assign(buffer, lengthKey);
  delete[] buffer;
}

//----------------------------------------------------------------------------------------------------------------------

inline void deserializeData(std::ifstream &file, int &ptr) {
  file.read(reinterpret_cast<char *>(&ptr), sizeof(ptr));
}

//----------------------------------------------------------------------------------------------------------------------

inline void deserializeData(std::ifstream &file, std::shared_ptr<certainData> &ptr, StringPool &strPull) {
  int id = 0;
  file.read(reinterpret_cast<char *>(&id), sizeof(id));

  int keyLength = 0;
  file.read(reinterpret_cast<char *>(&keyLength), sizeof(keyLength));

  char *buffer = new char[keyLength + 1];
  file.read(buffer, keyLength);
  buffer[keyLength] = '\0';

  std::string name;
  name.assign(buffer, keyLength);
  delete[] buffer;

  int extraDataLenght = 0;
  file.read(reinterpret_cast<char *>(&extraDataLenght), sizeof(extraDataLenght));
  std::unordered_map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > eData;

  for (int i = 0; i < extraDataLenght; ++i) {
    keyLength = 0;
    file.read(reinterpret_cast<char *>(&keyLength), sizeof(keyLength));
    buffer = new char[keyLength + 1];
    file.read(buffer, keyLength);
    buffer[keyLength] = '\0';

    std::string _key;
    _key.assign(buffer, keyLength);
    delete[] buffer;

    file.read(reinterpret_cast<char *>(&keyLength), sizeof(keyLength));
    buffer = new char[keyLength + 1];
    file.read(buffer, keyLength);
    buffer[keyLength] = '\0';

    std::string _value;
    _value.assign(buffer, keyLength);
    delete[] buffer;

    eData[std::make_shared<std::string>(_key)] = std::make_shared<std::string>(_value);
  }

  ptr = std::make_shared<certainData>(id, strPull.getString(name), eData);
}

//======================================================================================================================

inline std::wstring StringToWString(const std::string &str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
  return converter.from_bytes(str);
}

//----------------------------------------------------------------------------------------------------------------------

inline std::wstring CharToWstring(const CHAR *charStr) {
  if (charStr == nullptr) {
    return {};
  }

  // Получаем длину результирующей широкой строки
  int wideStrLen = MultiByteToWideChar(CP_ACP, 0, charStr, -1, nullptr, 0);
  if (wideStrLen == 0) {
    std::cerr << "MultiByteToWideChar failed to get length" << std::endl;
    return {};
  }

  // Создаем широкую строку
  std::wstring wideStr(wideStrLen - 1, L'\0'); // -1 чтобы убрать нулевой символ в конце

  // Преобразуем строку
  wideStrLen = MultiByteToWideChar(CP_ACP, 0, charStr, -1, &wideStr[0], wideStrLen);
  if (wideStrLen == 0) {
    std::cerr << "MultiByteToWideChar failed to convert" << std::endl;
    return {};
  }

  return wideStr;
}

//======================================================================================================================

inline bool deleteDirectory(const std::wstring &directory) {
  WIN32_FIND_DATA findFileData;
  HANDLE hFind = INVALID_HANDLE_VALUE;

  std::wstring directoryPattern = directory + L"\\*";
  hFind = FindFirstFileW(directoryPattern.c_str(), reinterpret_cast<LPWIN32_FIND_DATAW>(&findFileData));

  if (hFind == INVALID_HANDLE_VALUE) {
    std::wcerr << L"FindFirstFile failed for " << directory << std::endl;
    return false;
  }

  do {
    const std::wstring fileOrDir = CharToWstring(findFileData.cFileName);

    if (fileOrDir != L"." && fileOrDir != L"..") {
      std::wstring fullPath = directory + L"\\" + fileOrDir;

      if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        // Рекурсивно удаляем папки
        if (!deleteDirectory(fullPath)) {
          FindClose(hFind);
          return false;
        }
      } else {
        // Удаляем файлы
        if (!DeleteFileW(fullPath.c_str())) {
          std::wcerr << L"Failed to delete file: " << fullPath << std::endl;
          FindClose(hFind);
          return false;
        }
      }
    }
  } while (FindNextFile(hFind, &findFileData) != 0);

  FindClose(hFind);

  // Удаляем саму папку
  if (!RemoveDirectoryW(directory.c_str())) {
    std::wcerr << L"Failed to remove directory: " << directory << std::endl;
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------------------------------------

inline bool SafeDeleteFile(const std::wstring &filePath) {
  // Проверяем, существует ли файл
  if (GetFileAttributesW(filePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND) {
      std::wcerr << L"File not found: " << filePath << std::endl;
      return false;
    } else {
      std::wcerr << L"Error getting file attributes: " << filePath << L" (Error code: " << error << L")" << std::endl;
      return false;
    }
  }

  // Попытка удалить файл
  if (!DeleteFileW(filePath.c_str())) {
    DWORD error = GetLastError();
    std::wcerr << L"Failed to delete file: " << filePath << L" (Error code: " << error << L")" << std::endl;
    return false;
  }

  return true;
}

//======================================================================================================================

inline std::string removeFileName(const std::string &original, const std::string &substring) {
  std::string result = original; // создаем копию оригинальной строки
  size_t pos = result.find(substring); // ищем позицию подстроки

  if (pos != std::string::npos) {
    // если подстрока найдена
    result.erase(pos, substring.length()); // вырезаем подстроку
  } else {
    pos = result.find("node");

    if (pos != std::string::npos) {
      // если подстрока найдена
      result.erase(pos, result.length() - pos); // вырезаем подстроку
    }
  }

  return result; // возвращаем строку без подстроки
}

//======================================================================================================================
//======================================================================================================================
//======================================================================================================================


inline bool isNumber(const std::string &str) {
  std::stringstream ss(str);
  int number;
  char c;
  return !(ss >> number).fail() && !(ss >> c) && number >= 0;
  // Проверка на успешное преобразование и отсутствие лишних символов
}

//----------------------------------------------------------------------------------------------------------------------

inline std::string printFileToConsole(const std::string &filename) {
  std::ostringstream oss;
  std::ifstream file(filename); // Открытие файла для чтения

  if (!file.is_open()) {
    oss << "Error: Could not open file " << filename << '\n';
    return "";
  }

  std::string line;
  while (std::getline(file, line))
    oss << line << '\n';

  file.close();

  oss << "\nДля продолжения отправьте любую строку\n";

  return oss.str();
}

//----------------------------------------------------------------------------------------------------------------------

inline std::string getInput(const std::string &prompt) {
  std::string input;
  std::cout << prompt;
  std::getline(std::cin, input);
  return input;
}

//----------------------------------------------------------------------------------------------------------------------

inline bool isValidDateTime(const std::string &dateTime) {
  // Регулярное выражение для проверки формата <YYYY-MM-DD hh:mm:ss>
  const std::regex dateTimePattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");

  // Проверяем, соответствует ли строка формату
  if (!std::regex_match(dateTime, dateTimePattern)) {
    return false;
  }

  // Попытка разобрать строку с датой и временем
  std::istringstream iss(dateTime);
  std::tm tm = {};
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

  // Проверка, удалось ли разобрать строку и корректность даты и времени
  if (iss.fail() || !iss.eof()) {
    return false;
  }

  // Преобразование std::tm в std::time_t для проверки корректности даты
  std::time_t time = std::mktime(&tm);
  if (time == -1) {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------------------------------------

inline int findInsertIndex(const std::vector<std::string> &sortedVector, const std::string &timeString) {
  auto it = std::lower_bound(sortedVector.begin(), sortedVector.end(), timeString);

  return std::distance(sortedVector.begin(), it);
}

//----------------------------------------------------------------------------------------------------------------------

inline bool isValidBoolName(const std::string &boolChar) {
  if (boolChar.empty() || boolChar.length() > 1) return false;

  if (boolChar[0] != 'Y' && boolChar[0] != 'N')
    return false;

  return true;
}

//----------------------------------------------------------------------------------------------------------------------

// Зарезервированные имена файлов в Windows
const std::array<std::string, 22> RESERVED_NAMES = {
  "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7",
  "COM8", "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
};

// Функция проверки имени файла
inline bool isValidFileName(const std::string &filename) {
  // Проверка на пустую строку и длину имени файла (до 255 символов для пути)
  if (filename.empty() || filename.length() > 255) return false;

  // Проверка на зарезервированные имена
  std::string name = filename;
  std::ranges::transform(name, name.begin(), ::toupper); // Приведение к верхнему регистру

  if (std::ranges::find(RESERVED_NAMES, name) != RESERVED_NAMES.end())
    return false;

  // Проверка на недопустимые символы
  for (const char ch: filename) {
    if (ch == '/' || ch == '\\' || ch == ':' || ch == '*' || ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch ==
        '|')
      return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------------------------------------

// Функция проверки имени файла
inline bool isValidName(const std::string &name) {
  // Проверка на пустую строку и длину имени файла (до 255 символов для пути)
  if (name.empty() || name.length() > 255) return false;

  // Проверка на недопустимые символы
  for (const char ch: name)
    if (ch == '/' || ch == '\\' || ch == ':' || ch == '*' || ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch ==
        '|')
      return false;

  return true;
}

//----------------------------------------------------------------------------------------------------------------------

// Функция проверки имени файла
inline bool isValidType(const std::string &type) {
  if (type.empty()) return false;

  std::vector<std::string> validType = {"name", "id"};

  for (auto el: validType)
    if (el == type)
      return true;

  return false;
}

//======================================================================================================================

inline bool isFileEmpty(const std::string &filePath) {
  std::ifstream file(filePath);

  if (!file.is_open()) {
    std::cerr << "Ошибка открытия файла: " << filePath << std::endl;
    return false;
  }

  file.seekg(0, std::ios::end);

  std::streampos fileSize = file.tellg();

  file.close();

  return fileSize == 0;
}

//======================================================================================================================

inline uint32_t fnv1a32(const char *s) {
  uint32_t h = 0x811c9dc5;
  while (*s) {
    h ^= (uint8_t) *s++;
    h *= 0x01000193;
  }
  return h;
}

//----------------------------------------------------------------------------------------------------------------------

inline std::string calculate_file_hash(const std::string &file_path) {
  std::ifstream file(file_path, std::ios::binary | std::ios::in);
  if (!file.is_open()) {
    throw std::runtime_error("Can't open file: " + file_path);
  }

  std::string file_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  file.close();

  return std::to_string(fnv1a32(file_data.c_str()));
}

//----------------------------------------------------------------------------------------------------------------------

inline void loadFrom_fileHash(const std::string &_fileName,
                              std::vector<std::pair<std::string, std::string> > &fileHashes) {
  std::ifstream fileHashes_file("../server/Storage/fileHashes.dat", std::ios::binary);
  if (!fileHashes_file.is_open())
    return;

  if (!isFileEmpty(_fileName)) {
    std::string filename_cur, fileHash_cur;
    int i;

    deserializeData(fileHashes_file, i);

    while (i > 0) {
      deserializeData(fileHashes_file, filename_cur);
      deserializeData(fileHashes_file, fileHash_cur);

      fileHashes.emplace_back(filename_cur, fileHash_cur);
      --i;
    }
  }

  fileHashes_file.close();
}

//----------------------------------------------------------------------------------------------------------------------

inline void save_fileHash(const std::string &_fileName) {
  std::string fileHash = calculate_file_hash(_fileName);
  std::vector<std::pair<std::string, std::string> > fileHashes;
  std::string filename_cur, fileHash_cur;

  loadFrom_fileHash(_fileName, fileHashes);

  std::ofstream fileHashes_file("../server/Storage/fileHashes.dat", std::ios::binary);
  if (!fileHashes_file)
    return;

  bool is_updatedVec = false;
  for (auto & fileHashe : fileHashes) {
    if (fileHashe.first == _fileName) {
       is_updatedVec = true;
       fileHashe.second = fileHash;
       break;
    }
  }

  if (!is_updatedVec)
    fileHashes.emplace_back(_fileName, fileHash);

  int size = static_cast<int>(fileHashes.size());

  serializeData(fileHashes_file, size);

  while (size--) {
    serializeData(fileHashes_file, fileHashes[size].first);
    serializeData(fileHashes_file, fileHashes[size].second);
  }

  fileHashes_file.close();

}

//----------------------------------------------------------------------------------------------------------------------

inline bool check_fileHash(std::string &_fileName) {
  std::string fileHash = calculate_file_hash(_fileName);
  std::vector<std::pair<std::string, std::string> > fileHashes;
  std::string filename_cur, fileHash_cur;

  loadFrom_fileHash(_fileName, fileHashes);

  if (isFileEmpty(_fileName))
    return true;

  for (int i = static_cast<int>(fileHashes.size()); i >= 0; --i) {
    if (fileHashes[i].first == _fileName) {
      if (fileHashes[i].second != fileHash)
        return false;

      break;
    }
  }

  return true;
}

#endif //EXTRAFUNCTIONS_H
