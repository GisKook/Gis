#include "sqlite3.h"
#include "EngineSqliteDB/GKDatasetVectorSqlite.h"

using namespace GKBASE;
namespace GKENGINE{

void GKDatasetVectorSqlite::SetDataSource( GKDataSourceSqlite* pDataSource )
{
	m_pDataSource = pDataSource;
}

GKBASE::GKErrorCode GKDatasetVectorSqlite::SetName( const GKBASE::GKString& strDatasetName )
{
	GKDataset::SetName(strDatasetName);
	return GKSUCCESS;
}
}