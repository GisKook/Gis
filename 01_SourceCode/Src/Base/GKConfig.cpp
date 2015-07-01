#include "Base/GKConfig.h"

using namespace GKBASE;


GKConfig & GKBASE::GKConfig::GetInstance()
{
	static GKConfig instance;
	return instance; 
}