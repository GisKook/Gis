/*
	��װһЩ�㷨������,���ڰ���
	GetAvailableName(GKString ,std::map);
*/
#ifndef SOURCECODE_INCLUDE_TOOLKIT_GKALOGORITHM_H_H
#define SOURCECODE_INCLUDE_TOOLKIT_GKALOGORITHM_H_H

#include <map>
#include "Base/GKString.h"

namespace GKBASE{

// ��map�������ҵ����õ�����
template <typename T>
GKBASE::GKString& GetAvaliableName(GKBASE::GKString& str, const std::map<GKBASE::GKString, T>&);

}

#endif