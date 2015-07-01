/*
	GKByteString作为一个字节字符串，可以用来存储带编码的字符串
	采用std::string实现
*/
#ifndef SOURCECODE_INCLUDE_BASE_GKBYTESTRING_H_H
#define SOURCECODE_INCLUDE_BASE_GKBYTESTRING_H_H

#include <string>
#include "Base/GKDataType.h"
#include "Base/GKExport.h"

namespace GKBASE
{
// 字符集，作为一个字符串数组的索引


class BASE_API GKByteString
{
public:
	// 默认构造函数
	GKByteString(){};
	
	// 拷贝构造函数
	GKByteString(const GKByteString& str)
	{
		m_string = str.StdStr();
	};

	// 拷贝赋值函数
	GKByteString& operator=(const GKByteString& str)
	{
		this->m_string = str.StdStr();
		return *this;
	};

	// 根据std::string拷贝构造
	GKByteString(std::string str)
	{
		m_string=str;
	};

	// 根据const GKchar*拷贝构造
	GKByteString(const GKchar* pStr)
	{
		m_string = pStr;
	};

	// 得到长度
	inline GKint32 GetLength() const;

	// 得到const GKchar*内容
	inline const GKchar* Cstr() const;

	// 得到std::string的内容
	inline const std::string& StdStr() const;

	// 重载+=
	inline GKByteString& operator+=(const GKByteString& str); 

	// 得到字符编码
	inline GKBASE::GKCharset GetCharset() const;

	// 设置字符编码
	// param nCharset[in] 要设置的字符集编码
	inline void SetCharset(GKBASE::GKCharset nCharset); 

public:
	// 内部使用的string
	std::string m_string;
private:
	// 字符编码
	GKCharset	m_nCharset;
};

inline GKByteString& GKByteString::operator +=(const GKBASE::GKByteString &str) 
{
	this->m_string.append(str.Cstr(), str.GetLength());
	return *this;
}

inline const std::string& GKByteString::StdStr() const
{
	return m_string;
}

inline const GKchar* GKByteString::Cstr() const 
{
	return m_string.c_str();
}

inline GKint32 GKByteString::GetLength() const
{
	return this->m_string.length();
}

inline GKBASE::GKCharset GKByteString::GetCharset() const
{
	return m_nCharset;
}

inline void GKByteString::SetCharset(GKCharset nCharset) 
{
	m_nCharset = nCharset;
}

// 重载全局的+
// param str1[in] 字符串1
// param str2[in] 字符串2
BASE_API GKByteString operator+(const GKByteString& str1, const GKByteString& str2);

// 重载全局的==
// param str1[in] 字符串1
// param str2[in] 字符串2
BASE_API GKbool operator==(const GKByteString& str1, const GKByteString& str2);

// 重载全局的!=
// param str1[in] 字符串1
// param str2[in] 字符串2
BASE_API GKbool operator!=(const GKByteString& str1, const GKByteString& str2);


}

#endif