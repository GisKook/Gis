#ifndef GISKOOK_BASE_GKFILEPATH_H_H
#define GISKOOK_BASE_GKFILEPATH_H_H

#include "Base/GKDef.h"
#include "Base/GKExport.h"
#include "Base/GKDataType.h"
#include "Base/GKString.h"

NAMESPACEBEGIN(GKBASE)
class BASE_API GKFilePath{
public:
	GKFilePath(){};
	GKFilePath(const GKuchar * path):m_cstrlen(sizeof(m_cstr)){m_string = path;};

public:
	GKchar * locale_str();

	void SetCharset(GKBASE::GKCharset charset){m_charset = charset;}

	void ChangeSuffix(GKBASE::GKuchar * oldsuffix,GKBASE::GKuchar *newsuffix);

private:
	GKBASE::GKCharset m_charset;
	GKBASE::GKString m_string;
	GKBASE::GKchar m_cstr[255];
	GKBASE::GKint32 m_cstrlen;
};

NAMESPACEEND


#endif