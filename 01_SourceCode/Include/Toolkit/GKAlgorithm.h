/*
	封装一些算法在里面,现在包括
	GetAvailableName(GKString ,std::map);
*/
#ifndef SOURCECODE_INCLUDE_TOOLKIT_GKALOGORITHM_H_H
#define SOURCECODE_INCLUDE_TOOLKIT_GKALOGORITHM_H_H

#include <map>
#include "Base/GKString.h"

namespace GKBASE{

// 在map类型中找到可用的名称
template <typename T>
GKBASE::GKString& GetAvaliableName(GKBASE::GKString& str, const std::map<GKBASE::GKString, T>&);

}

#endif