#include "Engine/GKEngineToolkit.h"
#include "Engine/GKDataSource.h"

using namespace GKBASE;

namespace GKENGINE
{
GKBASE::GKErrorCode GKEngineToolkit::ExceSQL( GKDataSource* pDataSource, const GKBASE::GKString& strSQL)
{
	GKASSERT(pDataSource != NULL);
	GKEngineType nType = pDataSource->GetEngineType();
	GKBASE::GKErrorCode nReturnCode = GKSUCCESS;
	switch(nType)
	{
	case GKEngineType::SQLITEDB:
		{
			GKByteString strSqlUTF8;
			strSQL.ToUTF8(strSqlUTF8);
			sqlite3_stmt *pStmt = NULL;
			nReturnCode = sqlite3_prepare_v2(pDataSource->GetConnection(), strSqlUTF8.Cstr(), strSqlUTF8.GetLength(), &pStmt, NULL);
			GKASSERT(nReturnCode == SQLITE_OK);
			if (nReturnCode == SQLITE_OK)
			{
				nReturnCode = sqlite3_step(pStmt);
			}
			sqlite3_finalize(pStmt);
			return nReturnCode;
		}
		break;
	default:
		break;
	}
}
}