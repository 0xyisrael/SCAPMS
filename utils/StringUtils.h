#pragma once
#include <string>
#include <cctype>

namespace clinic {
inline std::string toLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
}
