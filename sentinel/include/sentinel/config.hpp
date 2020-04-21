
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define SENTINEL_PRINT_DEBUG

#define SENTINEL_APPLICATION_DIR       "sentinel"
#define SENTINEL_ENV_MODULES_DIRECTORY "SENTINEL_MODULES_DIRECTORY"
#define SENTINEL_CLIENT_LOAD_PROC      "sentinelclient_Load"
#define SENTINEL_CLIENT_UNLOAD_PROC    "sentinelclient_Unload"

#define SENTINEL_VECTOR_SMALL_NORM 0.001f

// ----------------------------
// SENTINEL API CONFIG

#ifdef SENTINEL_BUILD_DLL
    #define SENTINEL_API __declspec(dllexport)
#else
    #define SENTINEL_API __declspec(dllimport)
    #undef SENTINEL_PRINT_DEBUG
#endif // SENTINEL_BUILD_DLL

// ----------------------------
// SENTINEL DEBUG CONFIG

#ifdef SENTINEL_PRINT_DEBUG
    #include <cstdio>

    #define SENTINEL_DEBUG_VAR(fmt, var) std::printf("%-32s " fmt "\n", #var, var)
    #define SENTINEL_DEBUG_MESSAGE(fmt, ...) std::printf(fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define SENTINEL_DEBUG_VAR(fmt, var)
#endif // SENTINEL_PRINT_DEBUG

// ----------------------------
// COMPILER INTERFACE MACROS

#if defined(__GNUC__) or defined(__GNUG__)
    #define SENTINEL_ATTRIBUTES(...) __attribute__((__VA_ARGS__))
#else
    #define SENTINEL_ATTRIBUTES(...)
#endif
