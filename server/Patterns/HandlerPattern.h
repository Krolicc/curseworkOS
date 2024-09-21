#ifndef HANDLERPATTERN_H
#define HANDLERPATTERN_H

#include <sstream>

#include <iomanip>
#include <chrono>
#include <ctime>
#include <utility>

#include "../ExtraFunctions.h"

class Handler {
protected:
  std::shared_ptr<Handler> nextHandler;
  std::string commandName;
  std::shared_ptr<Command> revesrseCommand;
  std::string time;

public:
  Handler() : nextHandler(nullptr) {
  }

  virtual ~Handler() = default;

  void setNextHandler(std::shared_ptr<Handler> next) {
    nextHandler = std::move(next);
  }

  void registerTime() {
    std::ostringstream timeOss;

    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    const std::tm local_tm = *std::localtime(&now_time_t);

    timeOss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    time = timeOss.str();
  }

  void registerCommand(Command &command) {
    commandName = command.getName();
    revesrseCommand = command.getRevCommand();
  }

  void getHistory(std::ostringstream &oss) {
    if (nextHandler) {
      oss << "---- " << time << ": " << commandName << '\n';
      nextHandler->getHistory(oss);
    }
  }

  void back (const std::string &_time, std::vector<std::shared_ptr<Command> > &vec) {
    if (nextHandler) {
      if (_time < time) {
        vec.emplace_back(revesrseCommand);
      }

      nextHandler->back(_time, vec);
    } else {
      for (int i = vec.size() - 1; i >= 0; --i) {
        if (vec[i] != nullptr)
          handle(*vec[i]);
      }
    }
  }

  virtual void handle(Command &command) {
    if (!nextHandler) {
      registerTime();
      command.execute();
      registerCommand(command);
      setNextHandler(std::move(std::make_shared<Handler>()));
    } else {
      nextHandler->handle(command);
    }
  }
};

//======================================================================================================================

class HistoryHandler final : public Handler {
public:
  void handle(Command &command) override {
    Handler::handle(command);
  }

  [[nodiscard]] std::string printHistory() {
    std::ostringstream oss;
    std::ostringstream extra_oss;

    oss << "История команд:";

    getHistory(extra_oss);

    if (extra_oss.str().empty())
      oss << " Нет команд!" << '\n';
    else
      oss << '\n';

    oss << extra_oss.str();

    oss << "Для продолжения отправьте любую строку\n";

    return oss.str();
  }

  void getBack(const std::string &time) {
    std::vector<std::shared_ptr<Command> > vec;
    back(time, vec);
  }
};


#endif //HANDLERPATTERN_H
