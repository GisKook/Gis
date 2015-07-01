#include "unicode/ucnv.h"
#include "Base/GKTextEncoding.h"
#include "Base/GKSystem.h"
using namespace U_ICU_NAMESPACE;

namespace GKBASE
{
extern GKchar* g_pCharset[];

GKTextEncoding::GKTextEncoding( GKCharset nToCharset,GKCharset nFromCharset )
{
	m_bstrToCharset=g_pCharset[nToCharset];
	m_bstrFromCharset=g_pCharset[nFromCharset];
}

GKTextEncoding::GKTextEncoding( const GKByteString& bstrToCharset, const GKByteString& bstrFromCharset )
{
	m_bstrFromCharset = bstrFromCharset;
	m_bstrToCharset = bstrToCharset;
}

GKTextEncoding::GKTextEncoding( const GKString& strToCharset, const GKString& strFromCharset )
{
	strToCharset.ToUTF8(m_bstrToCharset);
	strFromCharset.ToUTF8(m_bstrFromCharset);
}

GKTextEncoding::~GKTextEncoding()
{
	 
}

GKbool GKTextEncoding::Convert( GKbyte* target, GKint32& nTargetLen, const GKbyte* source, GKint32 nSourceLen )
{
	GKASSERT(target != NULL && source != NULL);
	UErrorCode ErrorCode; 
	ucnv_convert(m_bstrToCharset.Cstr(), m_bstrToCharset.Cstr(), (char*)target, nTargetLen, (char*)source, nSourceLen,&ErrorCode);

	return ErrorCode == U_ZERO_ERROR; 
}

GKbool GKTextEncoding::Convert( GKString& str, const GKbyte* src, GKint32 nSrcLen )
{
// 	GKASSERT(target != NULL && source != NULL);
// 	UErrorCode ErrorCode; 
// 	ucnv_convert(m_bstrToCharset.Cstr(), m_bstrToCharset.Cstr(), (char*)target, nTargetLen, (char*)source, nSourceLen,&ErrorCode);
// 
// 
// 	return ErrorCode == U_ZERO_ERROR;
	return TRUE;
}
}


