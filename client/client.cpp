#include <iostream>
#include <sstream>

#include <string>

#include <windows.h>

const char *PIPE_NAME = R"(\\.\pipe\my_pipe)";

int main(const int argc, char *argv[]) {
  setlocale(LC_ALL, "Russian");

  if (argc != 2 || (strcmp(argv[1],"file") != 0 && strcmp(argv[1],"memory") != 0)) {
    std::cerr << "Используйте: " << argv[0] << " <storage_mode: file/memory> " << std::endl;
    return 1;
  }

  HANDLE hPipe;
  DWORD bytesWritten, bytesRead;
  CHAR buffer[4096];

  // Открытие именованного канала
  hPipe = CreateFileA(
    PIPE_NAME, // Имя канала
    GENERIC_READ | GENERIC_WRITE, // Доступ на чтение и запись
    0, // Совместный доступ (нет)
    nullptr, // Атрибуты безопасности
    OPEN_EXISTING, // Открыть существующий объект
    0, // Атрибуты файла
    nullptr // Шаблон файла
  );

  if (hPipe == INVALID_HANDLE_VALUE) {
    std::cerr << "Ошибка создания файла: " << GetLastError() << std::endl;
    return 1;
  }

  std::ostringstream oss;
  oss << argv[1];

  if (!WriteFile(hPipe, oss.str().c_str(), strlen(oss.str().c_str()), &bytesWritten, nullptr)) {
    std::cerr << "Ошибка отправки настроек: " << GetLastError() << std::endl;
    return 1;
  }

  std::string message_toServer;

  while (true) {
    if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
      buffer[bytesRead] = '\0';

      std::cout << "Получено от сервера: " << buffer << std::endl;
      std::getline(std::cin, message_toServer);
    } else {
      std::cerr << "Ошибка чтения: " << GetLastError() << std::endl;
      break;
    }

    // Отправка данных серверу
    if (!WriteFile(hPipe, message_toServer.c_str(), strlen(message_toServer.c_str()),
                   &bytesWritten, nullptr)) {
      std::cerr << "Ошибка записи: " << GetLastError() << std::endl;
      break;
    }

    if (strcmp(buffer, "End Of Program") == 0) {
      std::cout << "Получено от сервера: Завершение работы." << std::endl;
      break;
    }
  }

  CloseHandle(hPipe);

  return 0;
}
