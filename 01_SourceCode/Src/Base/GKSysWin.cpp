#include "Base/GKSystem.h"

#if defined (GK_WIN)
using namespace GKBASE;
GKBASE::GKCharset GKBASE::GetSystemDefaultLocal()
{ 
	int buffercount = 0;
	GKuchar localname[32] = {0};
	buffercount =  GetUserDefaultLocaleName(&localname, 32);
	return GKBASE::GB18030;
}
#endif