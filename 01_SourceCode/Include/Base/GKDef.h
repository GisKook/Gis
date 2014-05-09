// ����Щ���õĺ꣬��Ҫ�����C++���Ա����
#ifndef SOURCECODE_INCLUDE_BASE_GKDEFS_H_H
#define SOURCECODE_INCLUDE_BASE_GKDEFS_H_H

#include "gksystem.h"

// һЩ����
#define NULL 0
#define FALSE 0
#define TRUE 1


// һЩ��
#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    extern
#endif
#endif

// �ַ������
#define _U(s) L ## s

// ���ڿ���
#define U_SIZEOF_WCHAR_T 2

#define NAMESPACEBEGIN(namespace_name) namespace namespace_name{
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