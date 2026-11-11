#pragma once
#include <stdexcept>
#include <string>

namespace clinic {
class ClinicException : public std::runtime_error {
public:
    explicit ClinicException(const std::string& msg) : std::runtime_error(msg) {}
};
}
