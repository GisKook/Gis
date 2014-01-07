// 定义些常用的宏，主要是针对C++语言本身的
#ifndef SOURCECODE_INCLUDE_BASE_GKDEFS_H_H
#define SOURCECODE_INCLUDE_BASE_GKDEFS_H_H

#include "gksystem.h"

// 一些常量
#define NULL 0
#define FALSE 0
#define TRUE 1

#if ! defined(GK_WIN)
#if defined(_WIN32) || defined(WIN32)
#define GK_WIN
#endif // WIN32 or _WIN32
#else
#if defined(__linux__)	
#define GK_LUX
#endif // __linux__
#endif // GK_WIN

// 一些宏
#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    extern
#endif
#endif

// 字符串相关
#define _U(s) L ## s

// 用于控制
#define U_SIZEOF_WCHAR_T 2

#define NAMESPACEBEGIN(x) namespace x{
#define NAMESPACEEND }

#ifndef IN
#define IN
#endif

#ifndef OUT 
#define OUT 
#endif

#if defined __GNUC__
#define likely(x) __builtin_expect ((x), 1)
#define unlikely(x) __builtin_expect ((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#endif // SOURCECODE_INCLUDE_BASE_GKDEFS_H_H