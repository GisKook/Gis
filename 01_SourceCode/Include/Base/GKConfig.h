#ifndef GISKOOK_INCLUDE_BASE_GKCONFIG_H_H
#define GISKOOK_INCLUDE_BASE_GKCONFIG_H_H
#include "Base/GKDef.h"
#include "Base/GKExport.h"
NAMESPACEBEGIN(GKBASE)
class BASE_API GKConfig{
public:
	const GKConfig & GetInstance();

	void SetCharset(GKBASE::GKCharset charset){m_charset = charset;};
private:
	GKConfig();
	~GKConfig();
private:
	GKCharset m_charset; 
};
NAMESPACEEND
#endif