#include "Base/GKFilePath.h"
#include "Base/GKTextEncoding.h"

using namespace GKBASE;

GKchar * GKBASE::GKFilePath::locale_str(){ 
	//GKTextEncoding textencoding(m_charset, GKBASE::UTF16);
	//textencoding.Convert((GKbyte*)m_cstr, m_cstrlen,(GKbyte*)m_string.Cstr(), m_string.GetLength()); 
	//m_cstr[m_cstrlen] = 0;
	m_cstrlen = m_string.ToCharset(m_charset,m_cstr, m_cstrlen);
	m_cstr[m_cstrlen] = 0;

	return m_cstr;
}

void GKBASE::GKFilePath::ChangeSuffix( GKBASE::GKuchar *oldsuffix, GKBASE::GKuchar *newsuffix ) {
	m_string.Replace(oldsuffix, newsuffix);
}
