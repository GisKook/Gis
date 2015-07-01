/*�ַ�����ת��*/
#ifndef SOURCECODE_INCLUDE_BASE_GKTEXTENCODING_H_H
#define SOURCECODE_INCLUDE_BASE_GKTEXTENCODING_H_H

#include "Base/GKDataType.h"
#include "Base/GKString.h"

struct UConverter;
typedef struct UConverter UConverter;

namespace GKBASE
{


class BASE_API GKTextEncoding
{
public:
	GKTextEncoding(){};
	// ����ת����
	// param nToCharset[in] Ŀ�����
	// param nFromCharset[in] Դ����
	GKTextEncoding(GKCharset nToCharset,GKCharset nFromCharset);

	// ����ת����
	// param strToCharset[in] Ŀ�����
	// param strFromCharset[in] Դ����
	GKTextEncoding(const GKByteString& spToCharset, const GKByteString& spFromCharset);

	
	~GKTextEncoding();
public:
	void SetSrcCharset(GKCharset charset);
	
	void SetDstCharset(GKCharset charset);

public:
	// ����ת��
	// param target [out] Ŀ����������
	// param nTargetLen[out] Ŀ�����ĳ���
	// param source [in] Դ��������
	// param nSourceLen [in] ԭ���볤��
	GKbool Convert(GKbyte* target,
		GKint32& nTargetLen,
		const GKbyte* source,
		GKint32 nSourceLen);

private:
	// ICU---UConverter
	UConverter * m_dstConverter;
	UConverter * m_srcConverter;

	GKByteString m_dstCharset;
	GKByteString m_srcCharset;
};

}
#endif
