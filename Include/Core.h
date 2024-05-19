#ifndef __CORE_H__
#define __CORE_H__

#if defined(_WIN32) || defined(_WIN64)
#    define LIE_PLATFORM_WINDOWS
#elif defined(__linux__)
#    define LIE_PLATFORM_LINUX
#elif defined(__APPLE__)
#    define LIE_PLATFORM_MACOS
#else
#    error "Unsupported platform"
#endif

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t usize;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef ptrdiff_t isize;

typedef float f32;
typedef double f64;

#endif