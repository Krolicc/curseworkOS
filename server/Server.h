#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <sstream>

#include <windows.h>

#include "../server/ErrorHandler/ErrorThrower.h"

const char *PIPE_NAME = R"(\\.\pipe\my_pipe)";

class Server {
public:
  HANDLE hPipe;

  Server() = default;

  ~Server() {
    CloseHandle(hPipe);
  }

  std::string handleClientRequest();

  void sendResponse(const std::string &);

  std::string sendResponseAndGetResponse(const std::string &);

  void startListening();

  HANDLE &getPipe() {
    return hPipe;
  }
};

inline void Server::startListening() {
  CHAR buffer[1024], answer[1024];
  DWORD bytesRead, bytesWritten;

  setlocale(LC_ALL, "Russian");

  // �������� ������������ ������
  hPipe = CreateNamedPipeA(
    PIPE_NAME, // ��� ������
    PIPE_ACCESS_DUPLEX, // ������ �� ������ � ������
    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // ��� � �����
    1, // ������������ ���������� ���������
    sizeof(buffer), // ������ ������ ��� ������
    sizeof(buffer), // ������ ������ ��� ������
    0, // ����� ��������
    nullptr // �������� ������������
  );

  if (hPipe == INVALID_HANDLE_VALUE)
    throw BTreeError("������ �������� �������� ������: " + GetLastError() + '\n');

  std::cout << "�������� ����������� �����������..." << std::endl;

  if (!ConnectNamedPipe(hPipe, nullptr)) {
    CloseHandle(hPipe);
    throw BTreeError("������ ����������� ������������: " + GetLastError() + '\n');
  }
  std::cout << "����������� ���������." << std::endl;
}

inline void Server::sendResponse(const std::string &request) {
  DWORD bytesWritten;

  if (!WriteFile(hPipe, request.c_str(), strlen(request.c_str()), &bytesWritten, nullptr)) {
    throw BTreeError("������ ������: " + GetLastError() + '\n');
  }

  std::cout << "��������� ���� ���������� ������������" << std::endl;
}

inline std::string Server::handleClientRequest() {
  DWORD bytesRead;
  CHAR answer[4096];

  if (ReadFile(hPipe, answer, sizeof(answer) - 1, &bytesRead, nullptr)) {
    answer[bytesRead] = '\0'; // ��������� ������ ������� ��������
    std::cout << "�������� �� ������������: " << answer << std::endl;
  } else
    throw BTreeError("������ ������: " + GetLastError() + '\n');

  std::string res(answer);
  return std::move(res);
}

inline std::string Server::sendResponseAndGetResponse(const std::string &request) {
  DWORD bytesRead, bytesWritten;
  CHAR answer[4096];

  if (!WriteFile(hPipe, request.c_str(), strlen(request.c_str()), &bytesWritten, nullptr)) {
    throw BTreeError("������ ������: " + GetLastError() + '\n');
  }

  std::cout << "��������� ���� ���������� ������������" << std::endl;

  if (ReadFile(hPipe, answer, sizeof(answer) - 1, &bytesRead, nullptr)) {
    answer[bytesRead] = '\0'; // ��������� ������ ������� ��������
    std::cout << "�������� �� ������������: " << answer << std::endl;
  } else
    throw BTreeError("������ ������: " + GetLastError() + '\n');

  std::string res(answer);

  return std::move(res);
}


#endif //SERVER_H
