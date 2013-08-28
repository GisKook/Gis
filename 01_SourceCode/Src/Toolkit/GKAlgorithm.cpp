#include "Toolkit/GKAlgorithm.h"
#define SUFFIX _U("_1")

namespace GKBASE{

template <typename T>
GKBASE::GKString& GetAvaliableName(GKBASE::GKString& str, const std::map<GKBASE::GKString, T>& srcMap)
{
	if (strMap.find(str) != NULL)
	{
		str += SUFFIX;
		while(strMap.find(str) != NULL)
		{
			str += SUFFIX;
		}
	}
	
	return str;
}

}