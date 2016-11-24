#pragma once
#include <stdint.h>

#define cbSlow true
#define cbDebug true


#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT __declspec(dllexport)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)

#define cbInternal static
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Pi 3.14159265358979323846f
#define Pi_2 1.57079632679489661923f

#if cbSlow
#define Assert(Expression) if(!(Expression)) {__debugbreak();}
#else
#define Assert(Expression) Expression
#endif

typedef uint32_t uint;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef size_t mem_size;