#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H


#include <sstream>

#include "ExtraFunctions.h"

#include "BaseTree/BTree.h"
#include "BaseTree/CommonWrapper.h"

#include "ErrorHandler/ErrorThrower.h"

#include "Server.h"


inline void processFileCommands(const std::string &filename, CommonWrapper &data_manager, StringPool &string_pool,
                                std::ostringstream &oss, Server &server, HistoryHandler &history_handler) {
  int line_num = 0;

  std::ifstream file("../tests/" + filename + ".txt");

  if (file.fail()) {
    oss << "������ �������� �����\n��������� ������������ �������� ����� � ��� ������������ (����� tests)\n";
    return;
  }

  std::string line;

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string textCommand;
    iss >> textCommand;

    ++line_num;

    try {
      if (textCommand == "ADD_PULL") {
        std::string poolName;
        iss >> poolName;

        if (!isValidName(poolName))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        auto command = Add_Pool(poolName, data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "REMOVE_PULL") {
        std::string poolName;
        iss >> poolName;

        if (!isValidName(poolName))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        auto command = Remove_Pool(poolName, data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "ADD_SCHEMA") {
        std::string poolName, schemaName;
        iss >> poolName >> schemaName;

        if (!isValidName(poolName) || !isValidName(schemaName))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        auto command = Add_Schema(poolName, schemaName, data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "REMOVE_SCHEMA") {
        std::string poolName, schemaName;
        iss >> poolName >> schemaName;

        if (!isValidName(poolName) || !isValidName(schemaName))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        auto command = Remove_Schema(poolName, schemaName, data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "ADD_COLLECTION") {
        std::string poolName, schemaName, collectionName;
        iss >> poolName >> schemaName >> collectionName;

        if (!isValidName(poolName) || !isValidName(schemaName) || !isValidName(collectionName))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        auto command = Add_Collection(poolName, schemaName, collectionName, data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "REMOVE_COLLECTION") {
        std::string poolName, schemaName, collectionName;
        iss >> poolName >> schemaName >> collectionName;

        if (!isValidName(poolName) || !isValidName(schemaName) || !isValidName(collectionName))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        auto command = Remove_Collection(poolName, schemaName, collectionName, data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "ADD_DATA") {
        std::string poolName, schemaName, collectionName, id, name, pairsCount, mapKey, mapValue;
        std::map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > value;

        iss >> poolName >> schemaName >> collectionName >> id >> name >> pairsCount;

        if (!isValidName(poolName) || !isValidName(schemaName) || !isValidName(collectionName) || !isNumber(id)
            || !isNumber(pairsCount) || !isValidName(name))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        std::ostringstream add_oss;

        add_oss << id << ' ' << name << ' ' << pairsCount << ' ';

        std::stringstream ss(pairsCount);
        int count;
        ss >> count;

        while (count--) {
          iss >> mapKey >> mapValue;
          if (!isValidName(mapKey) || !isValidName(mapValue))
            break;
          add_oss << mapKey << ' ' << mapValue << ' ';
        }

        if (count > 0)
          throw BTreeError(
            "������������ �������������� ������!\n��������� �� ������������ � ���-�� � ��������� �������\n");

        auto command = Add_Data(poolName, schemaName, collectionName, data_manager, string_pool, add_oss.str());
        history_handler.handle(command);
      } else if (textCommand == "REMOVE_DATA") {
        std::string poolName, schemaName, collectionName, key;
        std::map<std::string, std::string> value;

        if (!isValidName(poolName) || !isValidName(schemaName) || !isValidName(collectionName) || !isNumber(key))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        iss >> poolName >> schemaName >> collectionName >> key;

        auto command = Remove_Data(poolName, schemaName, collectionName, data_manager, string_pool, key);
        history_handler.handle(command);
      } else if (textCommand == "UPDATE_DATA") {
        std::string poolName, schemaName, collectionName, id, name, pairsCount, mapKey, mapValue;
        std::map<std::shared_ptr<std::string>, std::shared_ptr<std::string> > value;

        iss >> poolName >> schemaName >> collectionName >> id >> name >> pairsCount;

        if (!isValidName(poolName) || !isValidName(schemaName) || !isValidName(collectionName) || !isNumber(id)
            || !isNumber(pairsCount) || !isValidName(name))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        std::ostringstream update_oss;

        update_oss << id << ' ' << name << ' ' << pairsCount;

        std::stringstream ss(pairsCount);
        int count;
        ss >> count;

        while (count--) {
          iss >> mapKey >> mapValue;
          if (!isValidName(mapKey) || !isValidName(mapValue))
            break;
          update_oss << ' ' << mapKey << ' ' << mapValue;
        }

        if (count > 0)
          throw BTreeError(
            "������������ �������������� ������!\n��������� �� ������������ � ���-�� � ��������� �������\n");

        auto command = Update_Data(poolName, schemaName, collectionName, data_manager, string_pool, update_oss.str());
        history_handler.handle(command);
      } else if (textCommand == "READ_ONE_DATA") {
        std::string poolName, schemaName, collectionName, key, type;
        std::map<std::string, std::string> value;

        iss >> poolName >> schemaName >> collectionName >> type >> key;

        if (!isValidName(poolName) || !isValidName(schemaName) || !isValidName(collectionName) || !isValidType(type) ||
            (type == "id" && !isNumber(key)) || (type == "name" && !isValidName(key)))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        auto command = ReadOne_Data(poolName, schemaName, collectionName, data_manager, string_pool, type, key, server);
        history_handler.handle(command);
      } else if (textCommand == "READ_RANGE_DATA") {
        std::string poolName, schemaName, collectionName, minKey, maxKey, type;
        std::map<std::string, std::string> value;

        iss >> poolName >> schemaName >> collectionName >> type >> minKey >> maxKey;

        if (!isValidName(poolName) || !isValidName(schemaName) || !isValidName(collectionName) || !isValidType(type) ||
            (type == "id" && (!isNumber(minKey) || !isNumber(maxKey))) || (
              type == "name" && (!isValidName(minKey) || !isValidName(maxKey))))
          throw BTreeError("������������ ���������!\n��������� �� ������������ � ��������� �������\n");

        auto command = ReadRange_Data(poolName, schemaName, collectionName, data_manager, string_pool, type, minKey,
                                      maxKey, server);
        history_handler.handle(command);
      } else if (textCommand == "//" || textCommand.empty()) {
        // COMMENT OR EMPTY LINE
      } else {
        throw BTreeError("�� ������ ������� � ������ = " + std::to_string(line_num) + "\n");
      }
    } catch (const BTreeError &e) {
      oss << e.what() << "������ " << std::to_string(line_num) << "\n";
      return;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------


inline void processUserCommands(CommonWrapper &data_manager, StringPool &string_pool, Server &server,
                                HistoryHandler &history_handler) {
  std::string textCommand;
  bool isCorrectCommand;

  std::string startMessage = "�������\n"
      "1. ���� �� ������ ��� ���������� � ����������� �����������;\n"
      "2. 'help', ����� �������� ������� �� ��������;\n"
      "3. 'history', ����� �������� ������� ����������� ������;\n"
      "4. 'GET_BACK', ����� ������� ����� ������� � ��������� ����������;\n"
      "5. 'exit', ����� ��������� � ����:";

  std::string regularMessage = "��� ������ ������:";

  textCommand = server.sendResponseAndGetResponse(startMessage);
  while (true) {
    isCorrectCommand = true;

    try {
      if (textCommand == "ADD_PULL" || textCommand == "AP") {
        std::string poolName;
        while (!isValidName(poolName))
          poolName = server.sendResponseAndGetResponse("������� <�������� ����>:");

        auto command = Add_Pool(poolName, data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "REMOVE_PULL" || textCommand == "RP") {
        std::string poolName;

        while (!isValidName(poolName))
          poolName = server.sendResponseAndGetResponse("������� <�������� ����>:");

        auto command = Remove_Pool(poolName, data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "ADD_SCHEMA" || textCommand == "AS") {
        std::string inputs[2];
        const std::string prompts[2] = {
          "������� <�������� ����>:",
          "������� <�������� �����>:",
        };

        for (int i = 0; i < 2; ++i)
          while (!isValidName(inputs[i]))
            inputs[i] = server.sendResponseAndGetResponse(prompts[i]);

        auto command = Add_Schema(inputs[0], inputs[1], data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "REMOVE_SCHEMA" || textCommand == "RS") {
        std::string inputs[2];
        const std::string prompts[2] = {
          "������� <�������� ����>:",
          "������� <�������� �����>:",
        };

        for (int i = 0; i < 2; ++i)
          while (!isValidName(inputs[i]))
            inputs[i] = server.sendResponseAndGetResponse(prompts[i]);

        auto command = Remove_Schema(inputs[0], inputs[1], data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "ADD_COLLECTION" || textCommand == "AC") {
        std::string inputs[3];
        const std::string prompts[3] = {
          "������� <�������� ����>:",
          "������� <�������� �����>:",
          "������� <�������� ���������>:",
        };

        for (int i = 0; i < 3; ++i)
          while (!isValidName(inputs[i]))
            inputs[i] = server.sendResponseAndGetResponse(prompts[i]);

        auto command = Add_Collection(inputs[0], inputs[1], inputs[2],
                                      data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "REMOVE_COLLECTION" || textCommand == "RC") {
        std::string inputs[3];
        const std::string prompts[3] = {
          "������� <�������� ����>:",
          "������� <�������� �����>:",
          "������� <�������� ���������>:",
        };

        for (int i = 0; i < 3; ++i)
          while (!isValidName(inputs[i]))
            inputs[i] = server.sendResponseAndGetResponse(prompts[i]);

        auto command = Remove_Collection(inputs[0], inputs[1], inputs[2],
                                         data_manager, string_pool);
        history_handler.handle(command);
      } else if (textCommand == "ADD_DATA" || textCommand == "AD") {
        std::string inputs[6];
        const std::string prompts[6] = {
          "������� <�������� ����>:",
          "������� <�������� �����>:",
          "������� <�������� ���������>:",
          "������� <id ������� ������>:",
          "������� <�������� ������� ������>:",
          "������� <���-�� ��� <����: ��������> >:",
        };

        std::ostringstream oss;

        for (int i = 0; i < 6; ++i)
          while ((i < 3 || i == 4) && !isValidFileName(inputs[i]) || (i == 3 || i == 5) && !isNumber(inputs[i]))
            inputs[i] = server.sendResponseAndGetResponse(prompts[i]);

        oss << inputs[3] << ' ' << inputs[4] << ' ' << inputs[5];

        if (isNumber(inputs[5])) {
          int pairsCount = stoi(inputs[5]), curPairNum = 1;
          std::string mapValue, mapKey;

          while (curPairNum <= pairsCount) {
            ++curPairNum;

            mapKey = server.sendResponseAndGetResponse("������� <�������� �����>:");
            mapValue = server.sendResponseAndGetResponse("������� <��������>:");

            oss << ' ' << mapKey << ' ' << mapValue;
          }

          auto command = Add_Data(inputs[0], inputs[1], inputs[2], data_manager,
                                  string_pool, oss.str());
          history_handler.handle(command);
        } else {
          std::cout << "������� �� �����" << std::endl;
        }
      } else if (textCommand == "REMOVE_DATA" || textCommand == "RD") {
        std::string inputs[4];
        const std::string prompts[4] = {
          "������� <�������� ����>:",
          "������� <�������� �����>:",
          "������� <�������� ���������>:",
          "������� <id ������� ������>:",
        };

        for (int i = 0; i < 4; ++i)
          while (i < 3 && !isValidFileName(inputs[i]) || i == 3 && !isNumber(inputs[i]))
            inputs[i] = server.sendResponseAndGetResponse(prompts[i]);

        auto command = Remove_Data(inputs[0], inputs[1], inputs[2], data_manager, string_pool, inputs[3]);
        history_handler.handle(command);
      } else if (textCommand == "UPDATE_DATA" || textCommand == "UD") {
        std::string inputs[6];
        const std::string prompts[6] = {
          "������� <�������� ����>:",
          "������� <�������� �����>:",
          "������� <�������� ���������>:",
          "������� <id ������� ������>:",
          "������� <�������� ������� ������>:",
          "������� <���-�� ��� <����: ��������> >:",
        };

        std::ostringstream oss;

        for (int i = 0; i < 6; ++i)
          while ((i < 3 || i == 4) && !isValidFileName(inputs[i]) || (i == 3 || i == 5) && !isNumber(inputs[i]))
            inputs[i] = server.sendResponseAndGetResponse(prompts[i]);

        oss << inputs[3] << ' ' << inputs[4] << ' ' << inputs[5];

        if (isNumber(inputs[5])) {
          int pairsCount = stoi(inputs[5]), curPairNum = 1;
          std::string mapValue, mapKey;

          while (curPairNum <= pairsCount) {
            ++curPairNum;

            mapKey = server.sendResponseAndGetResponse("������� <�������� �����>:");
            mapValue = server.sendResponseAndGetResponse("������� <��������>:");

            oss << ' ' << mapKey << ' ' << mapValue;
          }

          auto command = Update_Data(inputs[0], inputs[1], inputs[2],
                                     data_manager, string_pool, oss.str());
          history_handler.handle(command);
        } else {
          std::cout << "������� �� �����" << std::endl;
        }
      } else if (textCommand == "READ_RANGE_DATA" || textCommand == "RRD") {
        std::string inputs[6];
        const std::string prompts[6] = {
          "������� <�������� ����>:",
          "������� <�������� �����>:",
          "������� <�������� ���������>:",
          "������� <��� �����: 'id', 'name'>:",
          "������� <�������� ���. �����>:",
          "������� <�������� ����. �����>:",
        };

        for (int i = 0; i < 6; ++i)
          while (i < 3 && !isValidFileName(inputs[i]) || i == 3 && !isValidType(inputs[i]) || i > 3 && (
                   inputs[3] == "id" && !isNumber(inputs[i]) || inputs[3] == "name" && !isValidName(inputs[i])))
            inputs[i] = server.sendResponseAndGetResponse(prompts[i]);

        auto command = ReadRange_Data(inputs[0], inputs[1], inputs[2],
                                      data_manager, string_pool, inputs[3], inputs[4],
                                      inputs[5], server);
        history_handler.handle(command);
      } else if (textCommand == "READ_ONE_DATA" || textCommand == "ROD") {
        std::string inputs[5];
        const std::string prompts[5] = {
          "������� <�������� ����>:",
          "������� <�������� �����>:",
          "������� <�������� ���������>:",
          "������� <��� �����: 'id', 'name'>:",
          "������� <�������� �����>:",
        };

        for (int i = 0; i < 5; ++i)
          while (i < 3 && !isValidFileName(inputs[i]) || i == 3 && !isValidType(inputs[i]) || i == 4 && (
                   inputs[3] == "id" && !isNumber(inputs[i]) || inputs[3] == "name" && !isValidName(inputs[i])))
            inputs[i] = server.sendResponseAndGetResponse(prompts[i]);

        auto command = ReadOne_Data(inputs[0], inputs[1], inputs[2],
                                    data_manager, string_pool, inputs[3], inputs[4], server);

        history_handler.handle(command);
      } else if (textCommand == "history") {
        server.sendResponseAndGetResponse(history_handler.printHistory());
      } else if (textCommand == "GET_BACK" || textCommand == "back") {
        std::string time = server.sendResponseAndGetResponse("������� ���� � ������� <YYYY-MM-DD hh:mm:ss>:");

        if (isValidDateTime(time))
          history_handler.getBack(time);
        else
          throw BTreeError("������� �� ������ �����!\n��������� ������������ � ��������� �������\n");
      } else if (textCommand == "help") {
        server.sendResponseAndGetResponse(printFileToConsole("../server/help.txt"));
      } else if (textCommand == "exit") {
        break;
      } else {
        isCorrectCommand = false;
      }
    } catch (const BTreeError &e) {
      textCommand = server.sendResponseAndGetResponse(e.what() + regularMessage);
      continue;
    }

    std::string preMessage = isCorrectCommand
                               ? "Successful\n"
                               : "������������ ������ �������!\n���������� ������� �����, ������� ������� \"help\".\n";

    textCommand = server.sendResponseAndGetResponse(preMessage + regularMessage);
  }
}


#endif //COMMANDMANAGER_H
