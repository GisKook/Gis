/*
	GKByteString��Ϊһ���ֽ��ַ��������������洢��������ַ���
	����std::stringʵ��
*/
#ifndef SOURCECODE_INCLUDE_BASE_GKBYTESTRING_H_H
#define SOURCECODE_INCLUDE_BASE_GKBYTESTRING_H_H

#include <string>
#include "Base/GKDataType.h"
#include "Base/GKExport.h"

namespace GKBASE
{
// �ַ�������Ϊһ���ַ������������


class BASE_API GKByteString
{
public:
	// Ĭ�Ϲ��캯��
	GKByteString(){};
	
	// �������캯��
	GKByteString(const GKByteString& str)
	{
		m_string = str.StdStr();
	};

	// ������ֵ����
	GKByteString& operator=(const GKByteString& str)
	{
		this->m_string = str.StdStr();
		return *this;
	};

	// ����std::string��������
	GKByteString(std::string str)
	{
		m_string=str;
	};

	// ����const GKchar*��������
	GKByteString(const GKchar* pStr)
	{
		m_string = pStr;
	};

	// �õ�����
	inline GKint32 GetLength() const;

	// �õ�const GKchar*����
	inline const GKchar* Cstr() const;

	// �õ�std::string������
	inline const std::string& StdStr() const;

	// ����+=
	inline GKByteString& operator+=(const GKByteString& str); 

	// �õ��ַ�����
	inline GKBASE::GKCharset GetCharset() const;

	// �����ַ�����
	// param nCharset[in] Ҫ���õ��ַ�������
	inline void SetCharset(GKBASE::GKCharset nCharset); 

public:
	// �ڲ�ʹ�õ�string
	std::string m_string;
private:
	// �ַ�����
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

// ����ȫ�ֵ�+
// param str1[in] �ַ���1
// param str2[in] �ַ���2
BASE_API GKByteString operator+(const GKByteString& str1, const GKByteString& str2);

// ����ȫ�ֵ�==
// param str1[in] �ַ���1
// param str2[in] �ַ���2
BASE_API GKbool operator==(const GKByteString& str1, const GKByteString& str2);

// ����ȫ�ֵ�!=
// param str1[in] �ַ���1
// param str2[in] �ַ���2
BASE_API GKbool operator!=(const GKByteString& str1, const GKByteString& str2);


}

#endif