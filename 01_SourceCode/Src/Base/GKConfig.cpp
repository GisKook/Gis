#include "Base/GKConfig.h"

using namespace GKBASE;
const GKConfig & GKBASE::GKConfig::GetInstance(){
	static GKConfig instance; 
	return instance;
}
