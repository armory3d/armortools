#pragma once

#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32)
#define IRON_WINDOWS
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define IRON_IOS
#define IRON_APPLE_SOC
#else
#define IRON_MACOS
#if defined(__arm64__)
#define IRON_APPLE_SOC
#endif
#endif
#define IRON_POSIX
#elif defined(__linux__)
#if !defined(IRON_ANDROID)
#define IRON_LINUX
#endif
#define IRON_POSIX
#endif

int kickstart(int argc, char **argv);

typedef float    f32;
typedef double   f64;
typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
