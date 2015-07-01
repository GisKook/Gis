
#if defined (_WIN32)
#include <Windows.h>
#include "Base/GKSystem.h"
#include "Base/GKDataType.h"

using namespace GKBASE;
GKBASE::GKCharset GKBASE::GetSystemDefaultLocal()
{ 
	int buffercount = 0;
	GKuchar localname[32] = {0};
	buffercount = GetSystemDefaultLocaleName(localname, 32);

	return GKBASE::GB18030;
}
#endif