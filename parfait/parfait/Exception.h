#pragma once
#include <string>
#include <vector>
#include "StackTrace.h"

namespace Parfait {

class Exception : public std::exception {
  public:
    inline Exception(std::string e = "") : error_message(std::move(e)) {
        setStackTrace();
        std::string bang = "\u2757";
        error_message = bang + error_message + "\n" + trace + "\n\n" + bang + error_message;
    }
    inline const char* what() const noexcept override { return error_message.c_str(); }
    inline std::string stack_trace() const noexcept { return trace; }

    inline void appendMessage(std::string m) { error_message += "\n" + m; }

  private:
    std::string error_message;
    std::string trace;

    inline void setStackTrace() { trace = Parfait::stackTrace(); }
};
}
