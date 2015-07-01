/*�ַ�����ת��*/
#ifndef SOURCECODE_INCLUDE_BASE_GKTEXTENCODING_H_H
#define SOURCECODE_INCLUDE_BASE_GKTEXTENCODING_H_H

#include "Base/GKDataType.h"
#include "Base/GKString.h"

namespace GKBASE
{


class BASE_API GKTextEncoding
{
public:
	// ����ת����
	// param nToCharset[in] Ŀ�����
	// param nFromCharset[in] Դ����
	GKTextEncoding(GKCharset nToCharset,GKCharset nFromCharset);

	// ����ת����
	// param strToCharset[in] Ŀ�����
	// param strFromCharset[in] Դ����
	GKTextEncoding(const GKByteString& spToCharset, const GKByteString& spFromCharset);

	// ����ת����
	// param strToCharset[in] Ŀ�����
	// param strFromCharset[in] ԭ����
	GKTextEncoding(const GKString& strToCharset, const GKString& strFromCharset);

	~GKTextEncoding();
	
	// ����ת��
	// param target [out] Ŀ����������
	// param nTargetLen[out] Ŀ�����ĳ���
	// param source [in] Դ��������
	// param nSourceLen [in] ԭ���볤��
	GKbool Convert(GKbyte* target,
		GKint32& nTargetLen,
		const GKbyte* source,
		GKint32 nSourceLen);

	// ����ת��
	// param str[out] Ŀ��
	// 
	GKbool Convert(GKString& str, const GKbyte* src, GKint32 nSrcLen);
	
protected:
private:
	GKTextEncoding();
	GKTextEncoding(const GKTextEncoding&);
	GKTextEncoding& operator=(const GKTextEncoding&);

private:
	// ICU---UConverter
	void* m_pConvert;

	GKByteString m_bstrToCharset;
	GKByteString m_bstrFromCharset;
};

}
#endif
