#ifndef SOURCECODE_INCLUDE_ENGINE_GKDATASOURCEMANAGER_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKDATASOURCEMANAGER_H_H
#include "Base/GKDataType.h"
#include "Engine/GKDataSource.h"

namespace GKENGINE
{

class ENGINE_API GKDataSourceManager
{

public:
	GKDataSource* CreateDataSource(int eType){};

// 禁用构造等。
private:
	GKDataSourceManager(){};
	~GKDataSourceManager(){};
	GKDataSourceManager(const GKDataSourceManager& ){};
	GKDataSourceManager& operator=(const GKDataSourceManager&){return *this;};
};

}

#endif