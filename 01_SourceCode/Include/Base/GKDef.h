// ����Щ���õĺ꣬��Ҫ�����C++���Ա����
#ifndef SOURCECODE_INCLUDE_BASE_GKDEFS_H_H
#define SOURCECODE_INCLUDE_BASE_GKDEFS_H_H

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

#if ! defined(GK_WIN)
#if defined(_WIN32) || defined(WIN32)
#define GK_WIN
#endif // WIN32 or _WIN32
#else
#if defined(__linux__)	
#define GK_LUX
#endif // __linux__
#endif // GK_WIN

#if defined(GK_WIN)
static inline void gkassert( bool b )
{
	if (!b){
		__asm{ 
			INT 3
		}
	};
}

#define GKFPRINTF fprintf
#define GKASSERT gkassert
#else
#define GKASSERT assert
#endif

#define GKSPRINTF swprintf

#ifdef GK_WIN
// �����ļ�����
#define GKFOPEN _wfopen
#define GKFSEEK fseek
#define GKFTELL ftell
#define GKFREAD fread
#define GKFWRITE fwrite  
#define GKREMOVE _wremove 
#define GKACCESS _waccess
#endif

NAMESPACEBEGIN(GKBASE)
enum GKCharset{
	ANSI         =         0,
	DEFAULT      =         1,
	SYMBOL       =         2,
	GB2312		 =         3,
	GBK          =         4,
	GB18030      =         5,
	CHINESEBIG5  =         6,
	SHIFTJIS	 =         7, 
	HANGEUL      =         8,
	HANGUL       =         9,
	OEM			 =         10,
	JOHAB        =         11,
	HEBREW       =		   12,
	ARABIC       =         13,
	GREEK        =         14,
	TURKISH      =         15,
	VIETNAMESE   =         16,
	THAI         =         17,
	EASTEUROPE   =         18,
	RUSSIAN      =         19,
	MAC          =         20,
	BALTIC       =         21,
	UTF8         =         22, // Unicode �ַ�����һ�ֱ��뷽ʽ
	UTF16        =         23, // Unicode �ַ�����һ�ֱ��뷽ʽ
};
NAMESPACEEND
#endif // SOURCECODE_INCLUDE_BASE_GKDEFS_H_H