#ifndef LOGGER_H
#define LOGGER_H

#include <format>
#include <iostream>
#include <chrono>

#define STRIP_LOGGING 0

namespace logger {
    // 1 means bold
    static constexpr std::string reset = "\033[0m";
    static constexpr std::string red = "\033[1;31m";
    static constexpr std::string green = "\033[1;32m";
    static constexpr std::string yellow = "\033[1;33m";
    static constexpr std::string blue = "\033[1;34m";

    template <typename... Args>
    void log(std::string_view level, std::string_view color, std::format_string<Args...> fmt, Args&&... args) {
        auto now = std::chrono::zoned_time { "Europe/Rome", std::chrono::system_clock::now()}.get_local_time();
        auto now_s = std::chrono::time_point_cast<std::chrono::seconds>(now);
        auto value = now_s.time_since_epoch();
        long seconds = value.count();
        std::cout << std::format("{:%H:%M:}", now) << std::format("{:02}", seconds % 60) << " | "
            << "[" << color << level << reset << "] " << std::format(fmt, std::forward<Args>(args)...) << "\n";
    };

    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) {
        log("DEBUG", blue, fmt, std::forward<Args>(args)...);
    };

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        log("INFO", green, fmt, std::forward<Args>(args)...);
    };

    template <typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) {
        log("WARN", yellow, fmt, std::forward<Args>(args)...);
    };

    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        log("ERROR", red, fmt, std::forward<Args>(args)...);
    };
}

# if STRIP_LOGGING

#define LOG_INFO
#define LOG_DEBUG
#define LOG_WARN
#define LOG_ERROR

# else

#define LOG_INFO(...) logger::info(__VA_ARGS__)
#define LOG_DEBUG(...) logger::debug(__VA_ARGS__)
#define LOG_WARN(...) logger::warn(__VA_ARGS__)
#define LOG_ERROR(...) logger::error(__VA_ARGS__)

# endif

#endif // LOGGER_H
