#include "Engine/System.h"
#include <cstdlib>

bool System::isLinuxConsole() {
    const char* term = std::getenv("TERM");
    if (!term) return false;
    std::string t(term);
    return (t == "linux");
}
