#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

namespace string_utils
{
    std::vector<std::string> split_text(const std::string& message, size_t taille_max);
}

#endif
