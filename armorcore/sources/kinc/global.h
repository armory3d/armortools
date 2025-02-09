#pragma once

/*! \file global.h
    \brief Provides basic functionality that's used all over the place. There's usually no need to manually include this.
*/

#include <stdbool.h>
#include <stdint.h>

#define KINC_LITTLE_ENDIAN

#ifdef _MSC_VER
#define KINC_INLINE static __forceinline
#else
#define KINC_INLINE static __attribute__((always_inline)) inline
#endif

#ifdef _MSC_VER
#define KINC_MICROSOFT
#endif

#if defined(_WIN32)

#define KINC_WINDOWS

#elif defined(__APPLE__)

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE

#define KINC_IOS
#define KINC_APPLE_SOC

#else

#define KINC_MACOS

#if defined(__arm64__)
#define KINC_APPLE_SOC
#endif

#endif

#define KINC_POSIX

#elif defined(__linux__)

#if !defined(KINC_ANDROID)
#define KINC_LINUX
#endif

#define KINC_POSIX

#endif

int kickstart(int argc, char **argv);
