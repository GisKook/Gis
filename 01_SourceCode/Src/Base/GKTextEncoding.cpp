#include "unicode/ucnv.h"
#include "Base/GKTextEncoding.h"
#include "Base/GKSystem.h"
using namespace U_ICU_NAMESPACE;

namespace GKBASE
{
extern GKchar* g_pCharset[];

GKTextEncoding::GKTextEncoding( GKCharset nToCharset,GKCharset nFromCharset )
{
	m_dstCharset=g_pCharset[nToCharset];
	m_srcCharset=g_pCharset[nFromCharset];
}

GKTextEncoding::GKTextEncoding( const GKByteString& bstrToCharset, const GKByteString& bstrFromCharset )
{
	//ucnv_setDefaultName//
	m_srcCharset = bstrFromCharset;
	m_dstCharset = bstrToCharset;
}

GKTextEncoding::~GKTextEncoding()
{
	 
}

GKbool GKTextEncoding::Convert( GKbyte* target, GKint32& nTargetLen, const GKbyte* source, GKint32 nSourceLen )
{
	GKASSERT(target != NULL && source != NULL);
	nTargetLen = 0;
	UErrorCode ErrorCode;
	m_dstConverter = ucnv_open(m_dstCharset.Cstr(), &ErrorCode);
	if(U_FAILURE(ErrorCode)){
		ucnv_close(m_dstConverter); 
		return false;
	}
	m_srcConverter = ucnv_open(m_srcCharset.Cstr(), &ErrorCode);
	if(U_FAILURE(ErrorCode)){
		ucnv_close(m_srcConverter);
		return false;
	}
	
	ucnv_convert(m_dstCharset.Cstr(), m_dstCharset.Cstr(), (char*)target, nTargetLen, (char*)source, nSourceLen,&ErrorCode);
	ucnv_close(m_dstConverter);
	ucnv_close(m_srcConverter);

	return ErrorCode == U_ZERO_ERROR; 
}

void GKTextEncoding::SetSrcCharset( GKCharset charset ) {
	m_srcCharset = g_pCharset[charset];
}

void GKTextEncoding::SetDstCharset( GKCharset charset ) { 
	m_dstCharset = g_pCharset[charset];
}

}


