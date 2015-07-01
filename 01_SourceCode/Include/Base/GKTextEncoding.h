/*字符编码转换*/
#ifndef SOURCECODE_INCLUDE_BASE_GKTEXTENCODING_H_H
#define SOURCECODE_INCLUDE_BASE_GKTEXTENCODING_H_H

#include "Base/GKDataType.h"
#include "Base/GKString.h"

namespace GKBASE
{


class BASE_API GKTextEncoding
{
public:
	// 生成转换器
	// param nToCharset[in] 目标编码
	// param nFromCharset[in] 源编码
	GKTextEncoding(GKCharset nToCharset,GKCharset nFromCharset);

	// 生成转换器
	// param strToCharset[in] 目标编码
	// param strFromCharset[in] 源编码
	GKTextEncoding(const GKByteString& spToCharset, const GKByteString& spFromCharset);

	// 生成转换器
	// param strToCharset[in] 目标编码
	// param strFromCharset[in] 原编码
	GKTextEncoding(const GKString& strToCharset, const GKString& strFromCharset);

	~GKTextEncoding();
	
	// 编码转换
	// param target [out] 目标编码的内容
	// param nTargetLen[out] 目标编码的长度
	// param source [in] 源编码内容
	// param nSourceLen [in] 原编码长度
	GKbool Convert(GKbyte* target,
		GKint32& nTargetLen,
		const GKbyte* source,
		GKint32 nSourceLen);

	// 编码转换
	// param str[out] 目标
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
