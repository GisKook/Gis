/*
									基于ICU库的UNICODESTRING
*/
#ifndef SOURCECODE_INCLUDE_GKSTRING_H_H
#define SOURCECODE_INCLUDE_GKSTRING_H_H

#include "Base/GKDef.h.h"
#include "Base/GKDataType.h"
#include "Base/GKExport.h.h"
#include "Base/GKByteString.h"
#include <string>

namespace icu_44 
{
	class UnicodeString;
}
using namespace icu_44;

namespace GKBASE 
{



class BASE_API GKString
{
public:
	// brief 默认构造函数
	GKString();

	// brief 拷贝构造
	// param str[in] GKString类型
	GKString(const GKString& str);

	// brief 根据GKuchar*进行构造
	// param pStr[in] GKuchar*类型
	GKString(const GKuchar* pStr);
	
	// 根据字符串和长度构造
	// param pStr[in]
	// param nLenth[in]
	GKString(const GKuchar* pStr, GKint32 nLength);	

	// 根据UnicodeString构造
	// param str[in]UnicodeString
	GKString(const UnicodeString& str);

	// brief 赋值函数
	// param str[in] GKString类型
	GKString& operator=(const GKString& str);
	
	// brief 赋值函数
	// param pStr[in] UGuchar*类型
	GKString& operator=(const GKuchar* pStr);

	// 操作符 == 
	// param str[in] GKString类型 
	GKbool operator==(const GKString& str);

	// 操作符 ==
	// param pStr[in] GKuchar*类型
	GKbool operator==(const GKuchar* pStr);

	// 操作符 +=
	// param str[in] GKString类型
	GKString& operator+=(const GKString& str);

	// 操作符 +=
	// param pStr[in] GKuchar*类型
	GKString& operator+=(const GKuchar* pStr);

	// 操作符 != 
	// param str[in] GKString类型 
	GKbool operator!=(const GKString& str);

	// 操作符 <
	GKbool operator<(const GKString& str) const;

	// 操作符 >
	GKbool operator>=(const GKString& str) const;

	// 得到Cstr()
	const GKuchar* Cstr() const;

	// 得到长度
	GKint32 GetLength() const;
	
	// 得到内容
	// param nLength[int] 要得到的内容的长度
	// remark 必须和ReleaseBuffer匹配调用
	GKuchar* GetBuffer(GKint32 nLength);
	
	// 释放内容
	// param nLength长度如果是-1那么就是从遇到第一个NULL算为字符串长度
	// remark 必须与GetBuffer匹配调用
	void ReleaseBuffer(GKint32 nLength = -1);

	// 清空string里面的字符
	void Empty();

	// 赋值
	// param pStr; 要被复制的字符串
	// param nAssignLen; 要被复制的长度
	void Assign(const GKuchar* pStr, GKint32 nAssignLen);

	// 用GKchar*进行赋值
	// praram pStr[in] 要被赋值的字符串
	void Assign(const GKchar* pStr);

	// 用GKchar*的一段字符进行赋值
	// param pStr[in] 要被赋值的字符串
	// param nLen[in] 字符串的长度
	void Assign(const GKchar* pStr, GKint32 nLen);

	// 判断string是否为空
	GKbool IsEmpty() const;

	// 得到子字符串
	// param nStartPos[in] 起始位置
	// param nLen [in] 长度
	GKString SubString(GKint32 nStartPos, GKint32 nLen) const;

	// 得到UnicodeString的指针
	const UnicodeString* Custr() const;

	// 得到容量
	GKint32 GetCapacity() const;
 
	// 从UTF8转换到UTF16字符串
	// param pStr[in] UTF8字符串
	void FromUTF8(const GKchar* pStr) const;

	// 从UTF16转到UTF8
	// param [out]要输出的UTF8格式的string.
	GKByteString& ToUTF8(GKByteString& str) const;

	// 从指定编码的字符串得到UTF16编码的字符串
	// param strCharset[in] 字符集 
	// param Src[in] 要被转换的字符串
	GKString& FromByteString(const GKchar* strCharset, const GKchar* src);

	// 格式化文本
	GKString Format(const GKuchar* fmt, ...);

private:
	UnicodeString* m_pString;

private:
	
	// 这个结构是完全从ICU中抄过来的，目的是在调试的时候可以看到里面的内容，如果更新ICU库后，相应的
	// 结构也要改
 	struct
 	{
		enum
		{
			US_STACKBUF_SIZE= sizeof(void *)==4 ? 13 : 15, // Size of stack buffer for small strings
		};
 		void *    pvtable;
 		GKint8    nShortLength;   // 0..127: length  <0: real length is in fUnion.fFields.fLength
 		GKuint8   nFlags;         // bit flags: see constants above
 		union StackBufferOrFields 
 		{
 		// fStackBuffer is used iff (fFlags&kUsingStackBuffer)
 		// else fFields is used
 		GKuint16     fStackBuffer [US_STACKBUF_SIZE]; // buffer for small strings
 		struct 
 		{
 			GKuint16  fPadding;   // align the following field at 8B (32b pointers) or 12B (64b)
 			GKint32   fLength;    // number of characters in fArray if >127; else undefined
 			GKuint16  *fArray;    // the Unicode data (aligned at 12B (32b pointers) or 16B (64b))
 			GKint32   fCapacity;  // sizeof fArray
 		} fFields;
 		} fUnion;

 	}m_Context;
	
};

/**
* A string-like object that points to a sized piece of memory.
*
* We provide non-explicit singleton constructors so users can pass
* in a "const GKuchar*" or a "string" wherever a "StringPiece" is
* expected.
*
* Functions or methods may use const StringPiece& parameters to accept either
* a "const GKuchar*" or a "string" value that will be implicitly converted to
* a StringPiece.
*
* Systematic usage of StringPiece is encouraged as it will reduce unnecessary
* conversions from "const GKuchar*" to "string" and back again.
*
* @stable ICU 4.2
*/
class BASE_API StringPiece
{
private:
	const GKuchar*   m_pStr;
	GKint32       m_nLength;

public:
	/**
	* Default constructor, creates an empty StringPiece.
	* @stable ICU 4.2
	*/
	StringPiece() : m_pStr(NULL), m_nLength(0) { }
	/**
	* Constructs from a NUL-terminated const GKuchar* pointer.
	* @param str a NUL-terminated const GKuchar* pointer
	* @stable ICU 4.2
	*/
	StringPiece(const GKuchar* str);
	/**
	* Constructs from a std::string.
	* @stable ICU 4.2
	*/
//	StringPiece(const std::string& str)
//		: m_pStr(str.data()), m_nLength(static_cast<GKint32>(str.size())) { }

	/**
	* Constructs from a GKByteString.
	* @stable ICU 4.2
	*/
// 	StringPiece(const GKByteString& str)
// 		: m_pStr(str.m_string.data()), m_nLength(static_cast<GKint32>(str.m_string.size())){}

	/**
	* Constructs from a GKByteString.
	* @stable ICU 4.2
	*/
 	StringPiece(const GKString& str)
 		: m_pStr(str.Cstr()), m_nLength(static_cast<GKint32>(str.GetLength())){}
	/**
	* Constructs from a const GKuchar* pointer and a specified length.
	* @param offset a const GKuchar* pointer (need not be terminated)
	* @param len the length of the string; must be non-negative
	* @stable ICU 4.2
	*/
	StringPiece(const GKuchar* offset, GKint32 len) : m_pStr(offset), m_nLength(len) { }
	/**
	* Substring of another StringPiece.
	* @param x the other StringPiece
	* @param pos start position in x; must be non-negative and <= x.length().
	* @stable ICU 4.2
	*/
	StringPiece(const StringPiece& x, GKint32 pos);
	/**
	* Substring of another StringPiece.
	* @param x the other StringPiece
	* @param pos start position in x; must be non-negative and <= x.length().
	* @param len length of the substring;
	*            must be non-negative and will be pinned to at most x.length() - pos.
	* @stable ICU 4.2
	*/
	StringPiece(const StringPiece& x, GKint32 pos, GKint32 len);

	/**
	* Returns the string pointer. May be NULL if it is empty.
	*
	* data() may return a pointer to a buffer with embedded NULs, and the
	* returned buffer may or may not be null terminated.  Therefore it is
	* typically a mistake to pass data() to a routine that expects a NUL
	* terminated string.
	* @return the string pointer
	* @stable ICU 4.2
	*/
	const GKuchar* Data() const { return m_pStr; }
	/**
	* Returns the string length. Same as length().
	* @return the string length
	* @stable ICU 4.2
	*/
	GKint32 Size() const { return m_nLength; }
	/**
	* Returns the string length. Same as size().
	* @return the string length
	* @stable ICU 4.2
	*/
	GKint32 Length() const { return m_nLength; }
	/**
	* Returns whether the string is empty.
	* @return TRUE if the string is empty
	* @stable ICU 4.2
	*/
	GKbool IsEmpty() const { return m_nLength == 0; }

	/**
	* Sets to an empty string.
	* @stable ICU 4.2
	*/
	void Clear() { m_pStr = NULL; m_nLength = 0; }

	/**
	* Reset the stringpiece to refer to new data.
	* @param data pointer the new string data.  Need not be nul terminated.
	* @param len the length of the new data
	* @internal
	*/
	void Set(const GKuchar* data, GKint32 len) { m_pStr = data; m_nLength = len; }

	/**
	* Reset the stringpiece to refer to new data.
	* @param str a pointer to a NUL-terminated string. 
	* @internal
	*/
	void Set(const GKuchar* str);

	/**
	* Removes the first n string units.
	* @param n prefix length, must be non-negative and <=length()
	* @stable ICU 4.2
	*/
	void RemovePrefix(GKint32 n) {
		if (n >= 0) {
			if (n > m_nLength) {
				n = m_nLength;
			}
			m_pStr += n;
			m_nLength -= n;
		}
	}

	/**
	* Removes the last n string units.
	* @param n suffix length, must be non-negative and <=length()
	* @stable ICU 4.2
	*/
	void RemoveSuffix(GKint32 n) {
		if (n >= 0) {
			if (n <= m_nLength) {
				m_nLength -= n;
			} else {
				m_nLength = 0;
			}
		}
	}

	/**
	* Maximum integer, used as a default value for substring methods.
	* @stable ICU 4.2
	*/
	static const GKint32 npos;

	/**
	* Returns a substring of this StringPiece.
	* @param pos start position; must be non-negative and <= length().
	* @param len length of the substring;
	*            must be non-negative and will be pinned to at most length() - pos.
	* @return the substring StringPiece
	* @stable ICU 4.2
	*/
	StringPiece Substr(GKint32 pos, GKint32 len = npos) const {
		return StringPiece(*this, pos, len);
	}
};

/**
* Global operator == for StringPiece
* @param x The first StringPiece to compare.
* @param y The second StringPiece to compare.
* @return TRUE if the string data is equal
* @internal
*/
BASE_API GKbool  
	operator==(const StringPiece& x, const StringPiece& y);

/**
* Global operator != for StringPiece
* @param x The first StringPiece to compare.
* @param y The second StringPiece to compare.
* @return TRUE if the string data is not equal
* @internal
*/
inline GKbool operator!=(const StringPiece& x, const StringPiece& y) {
	return !(x == y);
}

// 针对GKString重载全局运算符+
// param str1[in]第一个str
// param str2[in]第二个str
BASE_API GKString operator+(const GKString& str1, const GKString& str2);

// 得到宽字符串的长度
BASE_API GKint32 GKStrLen(const GKuchar* str);

// 判断两个字符串是否相等
// retrun 如果相等返回0
BASE_API GKint32 GKStrCmp(const GKuchar* str1, GKuchar* str2);

// 判断两个字符串是否相等，不区分大小写
// return 如果相等返回0
BASE_API GKint32 GKStrCmpNoCase(const GKuchar* str1, const GKuchar* str2);

// 格式化文本
// buf[out] 格式化后文本的内容
// fmt[in] 格式化文本的格式
// args[in] 参数列表
// 如果函数格式化成功，返回格式化后buf的大小，如果失败，返回负值
GKint32 GKPrintf(GKuchar* buf, const GKuchar* fmt, va_list args);

#define GKSTRLEN(s) GKStrLen(s)
#define GKSTRALEN(s) strlen(s)
#define GKSTRCMP(s1, s2) GKStrCmp(s1, s2)
#define GKSTRCMPNOCASE(s1, s2) GKStrCmpNoCase(s1, s2)
#define GKSTRCPY(des, src) wcscpy(des, src)
#define GKSTRNCPY(des, src, size) wcsncpy(des, src, size)
#define GKSTRCHR(s, c) wcschr(s, c)
#define GKSTRSTR(s, c) wcsstr(s, c)
#define GKSTRTOL(str, end, base) wcstol(str, end, base)
#define GKMEMCMP(str1, str2, size) memcmp(str1, str2, size)
#define GKSTRCMPA(s1, s2) strcmp(s1, s2)
#define GKSTRLENA(s) strlen(s)
#define GKISDIGIT(c) (c>=_U('0') && c<=_U('9'))
#define GKISASCII(c) (c>=_U('a') && c<=_U('z') || c>=_U('A')&&c<=_U('Z'))

}
#endif //SOURCECODE_INCLUDE_GKSTRING_H_H