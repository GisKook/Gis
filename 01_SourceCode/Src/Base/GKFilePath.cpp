#include "Base/GKFilePath.h"
#include "Base/GKTextEncoding.h"

using namespace GKBASE;

GKchar * GKBASE::GKFilePath::locale_str(){ 
	GKTextEncoding textencoding(m_charset, GKBASE::UTF16);
	textencoding.Convert((GKbyte*)m_cstr, m_cstrlen,(GKbyte*)m_string.Cstr(), m_string.GetLength()); 
	m_cstr[m_cstrlen] = 0;
	return m_cstr;
}

void GKBASE::GKFilePath::ChangeSuffix( GKBASE::GKuchar *suffix ) { 
	GKint32 strlen = m_string.GetLength();
	for(GKint32 i = strlen - 1; i > 0; ++i){
		if(m_string[i] == _U(".")){
		}
	}
}
