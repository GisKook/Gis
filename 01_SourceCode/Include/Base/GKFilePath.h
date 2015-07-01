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
	GKFilePath(const GKuchar * path){m_string = path;};

public:
	GKchar * locale_str();

private:
	GKBASE::GKCharset m_charset;
	GKBASE::GKString m_string;
};

NAMESPACEEND


#endif