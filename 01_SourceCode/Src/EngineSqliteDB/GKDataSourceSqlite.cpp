#include "EngineSqliteDB/GKDataSourceSqlite.h"
#include "EngineSqliteDB/GKDatasetVectorSqlite.h"
#include "Base/GKString.h"
#include "sqlite3.h"
#include "Toolkit/GKFile.h"
#include "Base/GKErrors.h"
#include "Engine/GKEngineToolkit.h"

using namespace GKBASE;

namespace GKENGINE 
{
ENGINESQLITEDB_API extern GKBASE::GKString g_SQLsqlite[] = {
	_U("create table GKREG(") 
	_U("TID integer primary key,")
	_U("DTNAME nvarchar(128),")
	_U("TNAME nvarchar(128),")
	_U("SRID integer")
};

GKDataSourceSqlite::GKDataSourceSqlite()
{
	//sqlite3_initialize();
	m_pHandle = NULL;
}

GKBASE::GKErrorCode GKDataSourceSqlite::Open(const GKBASE::GKString& strFilePath, unsigned int nFlags )
{
	sqlite3* pHandle = NULL;
	GKBASE::GKByteString strFilePathUTF8;
	strFilePath.ToUTF8(strFilePathUTF8);

	GKBASE::GKErrorCode nErrorCode = sqlite3_open_v2(strFilePathUTF8.Cstr(), &pHandle, nFlags, NULL);
	GKASSERT(SQLITE_OK == nErrorCode);
	m_pHandle = pHandle;
	
	return nErrorCode;
}

GKBASE::GKErrorCode GKDataSourceSqlite::Close() const
{
	return sqlite3_close(m_pHandle);
}

GKBASE::GKErrorCode GKDataSourceSqlite::Delete( const GKBASE::GKString& strFilePath) const
{
	return GKFile::Delete(strFilePath);
}

GKDataSourceSqlite::~GKDataSourceSqlite()
{
	sqlite3_shutdown();
}

GKBASE::GKErrorCode GKDataSourceSqlite::Connect( const GKDataSourceConnInfo& DsConnInfo )
{
	if (Open(DsConnInfo.strServer, OPEN_READWRITE))
	{


	}
	else
	{
		return EDSS_OPENFAILE;
	}

	return GKSUCCESS;
}

GKBASE::GKErrorCode GKDataSourceSqlite::Create( const GKBASE::GKString& strPath, GKbool bOverWrite ) 
{
	GKBASE::GKErrorCode nReturnCode = GKSUCCESS;
	if (!GKFile::IsExist(strPath))
	{
		nReturnCode = Open(strPath, OPEN_READWRITE | OPEN_CREATE);
	}
	else if(bOverWrite)
	{
		GKFile::Delete(strPath);
		nReturnCode = Open(strPath, OPEN_READWRITE| OPEN_CREATE);
	}
	else
	{
		return EDSS_FILEEXIST;
	}

	if (nReturnCode == GKSUCCESS)
	{
		nReturnCode = GKEngineToolkit::GetInstance().ExceSQL(this, g_SQLsqlite[0]);
	}
	return nReturnCode;
}

sqlite3* GKDataSourceSqlite::GetConnection() const
{
	return m_pHandle;
}

GKDataset* GKDataSourceSqlite::CreateDataset( GKDatasetInfo* pDatasetInfo )
{
	GKDataset* pDataset = NULL;
	GKuint32 nType = pDatasetInfo->nType;
	switch (nType)
	{
	case GKENGINE::LINE:
	case GKENGINE::POINT:
	case GKENGINE::REGION:
		{
			GKString strName = ((GKDatasetVectorInfo*)pDatasetInfo)->strName;
			strName = GetAvaliableName(strName);
			pDataset = new GKDatasetVectorSqlite;
			pDataset->SetName(strName);
	#pragma  message("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~操作数据库了~~~~~~~~~~~~~~~~~~~~~~~~")

			if (GetDatasetCount() == 0)
			{
				((GKDatasetVectorSqlite*)pDataset)->ExecSql(g_SQLsqlite[0]);
			}

			AddDataset(pDataset);
		}
	default:
		return NULL;
		break;
	}

	return NULL;
}
}




