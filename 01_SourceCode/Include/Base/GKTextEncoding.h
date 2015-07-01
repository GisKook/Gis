/*字符编码转换*/
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
	// 生成转换器
	// param nToCharset[in] 目标编码
	// param nFromCharset[in] 源编码
	GKTextEncoding(GKCharset nToCharset,GKCharset nFromCharset);

	// 生成转换器
	// param strToCharset[in] 目标编码
	// param strFromCharset[in] 源编码
	GKTextEncoding(const GKByteString& spToCharset, const GKByteString& spFromCharset);

	
	~GKTextEncoding();
public:
	void SetSrcCharset(GKCharset charset);
	
	void SetDstCharset(GKCharset charset);

public:
	// 编码转换
	// param target [out] 目标编码的内容
	// param nTargetLen[out] 目标编码的长度
	// param source [in] 源编码内容
	// param nSourceLen [in] 原编码长度
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
