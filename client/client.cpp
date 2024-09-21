#include <iostream>
#include <sstream>

#include <string>

#include <windows.h>

const char *PIPE_NAME = R"(\\.\pipe\my_pipe)";

int main(const int argc, char *argv[]) {
  setlocale(LC_ALL, "Russian");

  if (argc != 2 || (strcmp(argv[1],"file") != 0 && strcmp(argv[1],"memory") != 0)) {
    std::cerr << "�����������: " << argv[0] << " <storage_mode: file/memory> " << std::endl;
    return 1;
  }

  HANDLE hPipe;
  DWORD bytesWritten, bytesRead;
  CHAR buffer[4096];

  // �������� ������������ ������
  hPipe = CreateFileA(
    PIPE_NAME, // ��� ������
    GENERIC_READ | GENERIC_WRITE, // ������ �� ������ � ������
    0, // ���������� ������ (���)
    nullptr, // �������� ������������
    OPEN_EXISTING, // ������� ������������ ������
    0, // �������� �����
    nullptr // ������ �����
  );

  if (hPipe == INVALID_HANDLE_VALUE) {
    std::cerr << "������ �������� �����: " << GetLastError() << std::endl;
    return 1;
  }

  std::ostringstream oss;
  oss << argv[1];

  if (!WriteFile(hPipe, oss.str().c_str(), strlen(oss.str().c_str()), &bytesWritten, nullptr)) {
    std::cerr << "������ �������� ��������: " << GetLastError() << std::endl;
    return 1;
  }

  std::string message_toServer;

  while (true) {
    if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
      buffer[bytesRead] = '\0';

      std::cout << "�������� �� �������: " << buffer << std::endl;
      std::getline(std::cin, message_toServer);
    } else {
      std::cerr << "������ ������: " << GetLastError() << std::endl;
      break;
    }

    // �������� ������ �������
    if (!WriteFile(hPipe, message_toServer.c_str(), strlen(message_toServer.c_str()),
                   &bytesWritten, nullptr)) {
      std::cerr << "������ ������: " << GetLastError() << std::endl;
      break;
    }

    if (strcmp(buffer, "End Of Program") == 0) {
      std::cout << "�������� �� �������: ���������� ������." << std::endl;
      break;
    }
  }

  CloseHandle(hPipe);

  return 0;
}
