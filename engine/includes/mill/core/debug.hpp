#pragma once

#include <spdlog/spdlog.h>

#define LOG_TRACE(...) ::spdlog::trace(__VA_ARGS__)
#define LOG_DEBUG(...) ::spdlog::debug(__VA_ARGS__)
#define LOG_INFO(...) ::spdlog::info(__VA_ARGS__)
#define LOG_WARN(...) ::spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...) ::spdlog::error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::spdlog::critical(__VA_ARGS__)

#if MILL_WINDOWS
    #define MILL_DEBUG_BREAK() __debugbreak()
#elif MILL_LINUX
    #include <signal.h>
    #ifdef SIGTRAP
        #define MILL_DEBUG_BREAK() raise(SIGTRAP)
    #else
        #define MILL_DEBUG_BREAK() raise(SIGABRT)
    #endif
#else
    #define MILL_DEBUG_BREAK()
#endif

#define ASSERT(_expr)                                                                  \
    do                                                                                 \
    {                                                                                  \
        if (!(_expr))                                                                  \
        {                                                                              \
            LOG_CRITICAL("Assert Failed: {} at {}:{}", "##_expr", __FILE__, __LINE__); \
            MILL_DEBUG_BREAK();                                                        \
        }                                                                              \
    } while (false)