#pragma once

#include <stdexcept>
#include <fmt/format.h>

enum ErrorCode {
    CODE_OK = 0,
    CODE_EOF = 100,
    CODE_EXCEPTION = 150,
    CODE_OPEN = 200,
    CODE_CLOSE = 300,
    CODE_SHUTDOWN = 400,
};

class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string &msg, int code = 0, int innerCode = 0);
    
    // 显式支持拷贝和移动语义，消除 MSVC C5272 警告
    Exception(const Exception&) = default;
    Exception& operator=(const Exception&) = default;
    Exception(Exception &&) noexcept = default;
    Exception& operator=(Exception &&) noexcept = default;

    ~Exception() override = default;

    const char *what() const noexcept override;

    int code() const;
    int innerCode() const;

private:
    int _code = 0;
    int _innerCode = 0;
    std::string _what;
};

inline Exception::Exception(const std::string &msg, int code, int innerCode)
    : std::runtime_error(msg), _code(code), _innerCode(innerCode) {
    _what = fmt::format("{} (error code: {})", msg, code);
}

inline const char * Exception::what() const noexcept {
    return _what.c_str();
}

inline int Exception::code() const {
    return _code;
}

inline int Exception::innerCode() const {
    return _innerCode;
}
