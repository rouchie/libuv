#pragma once

#include <stdexcept>
#include <fmt/format.h>

class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string &msg, int code = 0);
    ~Exception() override = default;

    const char *what() const noexcept override;

    int code() const;

private:
    int _code = 0;
};

inline Exception::Exception(const std::string &msg, int code)
    : std::runtime_error(msg), _code(code) {
}

inline const char * Exception::what() const noexcept {
    return fmt::format("{} (error code: {})", runtime_error::what(), _code).c_str();
}

inline int Exception::code() const {
    return _code;
}
