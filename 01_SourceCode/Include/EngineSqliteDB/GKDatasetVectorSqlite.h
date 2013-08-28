/*
	直接从GKDataset继承,中间就不搞什么DatasetVector了.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKDATASETVECTORSQLITE_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKDATASETVECTORSQLITE_H_H

#include "Engine/GKDataset.h"
#include "EngineSqliteDB/GKDataSourceSqlite.h"

namespace GKENGINE{

class ENGINESQLITEDB_API GKDatasetVectorSqlite : public GKDataset
{
	friend class GKDataSourceSqlite;
public:
	// 设置名字
	// strDatasetName[in] 数据集名字
	// 返回错误码
	virtual GKBASE::GKErrorCode SetName(const GKBASE::GKString& strDatasetName);

public:

	// 设置数据源
	void SetDataSource(GKDataSourceSqlite* pDataSource);

private:
	GKDatasetVectorSqlite(){};
	~GKDatasetVectorSqlite(){};

	};

}
#endif