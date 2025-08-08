#pragma once

#ifndef LOG_ENABLED
#define LOG_ENABLED 0
#endif

#include <string>
#include "Logger.h"

template <bool Enabled>
struct LoggerT {
    static inline void start() {}
    static inline void stop() {}
    static inline void log(const std::string&) {}
};

template <>
struct LoggerT<true> {
    static inline void start() { Logger::getInstance().start(); }
    static inline void stop() { Logger::getInstance().stop(); }
    static inline void log(const std::string& message) { Logger::getInstance().log(message); }
};