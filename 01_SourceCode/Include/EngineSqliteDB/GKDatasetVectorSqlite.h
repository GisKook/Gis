/*
	ֱ�Ӵ�GKDataset�̳�,�м�Ͳ���ʲôDatasetVector��.
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
	// ��������
	// strDatasetName[in] ���ݼ�����
	// ���ش�����
	virtual GKBASE::GKErrorCode SetName(const GKBASE::GKString& strDatasetName);

public:

	// ��������Դ
	void SetDataSource(GKDataSourceSqlite* pDataSource);

private:
	GKDatasetVectorSqlite(){};
	~GKDatasetVectorSqlite(){};

	};

}
#endif