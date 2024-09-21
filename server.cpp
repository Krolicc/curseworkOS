#include <iostream>
#include <sstream>

#include <windows.h>

#include "server/Patterns/SingletonFlyweightPattern.h"

#include "server/Patterns/CommandPattern.h"
#include "server/Patterns/HandlerPattern.h"
#include "server/ProcessCommands.h"

#include "server/BaseTree/CommonWrapper.h"

#include "server/Server.h"

//----------------------------------------------------------------------------------------------------------------------


int main() {
  Server server;

  server.startListening();

  std::ostringstream storageMode;
  storageMode << server.handleClientRequest();

  const bool storage_isFile = storageMode.str() == "file";

  std::string message = "������ �� �� ��������� ������ � �������� �������: Y / N";
  std::string savingAndLoading;

  while (!storage_isFile && !isValidBoolName(savingAndLoading)) {
    savingAndLoading = server.sendResponseAndGetResponse(message);

    if (!isValidBoolName(savingAndLoading))
      message = "������������ �����: " + savingAndLoading + "\n������� ������ �� �� ��������� ������: Y / N";
  }

  //----------------------------------------------------------------------------------------------------------------------

  bool _loadingAndSaving_bool = savingAndLoading == "Y";
  message = "������� �������� ����� ��������:";
  std::string storageName;

  while ((storageMode.str() == "file" || _loadingAndSaving_bool) && !isValidFileName(storageName)) {
    storageName = server.sendResponseAndGetResponse(message);

    if (!isValidFileName(storageName))
      message = "������������ ��� �����: " + storageName + "\n������� �������� ����� ��������:";
  }

  //----------------------------------------------------------------------------------------------------------------------

  HistoryHandler history_handler;
  StringPool &stringPool = StringPool::getInstance();
  CommonWrapper data_manager(!storage_isFile, storageName, stringPool, server, _loadingAndSaving_bool);

  std::string usageMode = "�� ������ �������� � ������ 'file', 'user' ��� ����� 'exit':";

  while (true) {
    server.sendResponse(usageMode);

    std::string userRequest = server.handleClientRequest();

    if (userRequest == "file") {
      std::string fileName = server.sendResponseAndGetResponse("������� �������� ������������� �����:");
      std::ostringstream oss;

      if (!isValidFileName(fileName)) {
        oss << "������������ ��� �����: " << fileName << "\n��������� ������������ ��� ����� � ��������� �������" << std::endl;
      } else
        processFileCommands(fileName, data_manager, stringPool, oss, server, history_handler);

      if (oss.str().empty())
        oss << "Successful\n";

      oss << "��� ����������� ������� ����� ������";

      server.sendResponseAndGetResponse(oss.str());
    } else if (userRequest == "user") {
      processUserCommands(data_manager, stringPool, server, history_handler);
    } else if (userRequest == "exit") {
      std::cout << "���������� ������ �������" << std::endl;
      server.sendResponseAndGetResponse("End Of Program");
      break;
    }
    usageMode = "��� ����� ������ ������? ������� 'file', 'user' ��� 'exit':";
  }

  return 0;
}