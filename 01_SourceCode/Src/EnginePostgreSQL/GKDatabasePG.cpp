#include "EnginePostgreSQL/GKDatabasePG.h"
#include "libpq-fe.h"
#include "EnginePostgreSQL/GKRecordsetPG.h"

using namespace GKBASE;
NAMESPACEBEGIN(GKENGINE)


GKBASE::GKErrorCode GKENGINE::GKDatabasePG::Connect( const GKDBConnInfo& dbConnInfo ) {
	GKASSERT(dbConnInfo.pgConn.type==GKENGINE::GKPOSTGRESQL); 
	PGconn *conn= PQsetdbLogin(dbConnInfo.pgConn.pghost, dbConnInfo.pgConn.pgport,
		dbConnInfo.pgConn.pgoptions, dbConnInfo.pgConn.pgtty, dbConnInfo.pgConn.dbName, 
		dbConnInfo.pgConn.login, dbConnInfo.pgConn.passwd);
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
			PQerrorMessage(conn));
	    PQfinish(conn);

		return E_ENGINE_CONNERROR;
	}
	m_pConnection=conn;
	m_nType = dbConnInfo.pgConn.type;

	return 0;
}

GKBASE::GKErrorCode GKDatabasePG::DisConnect() {
	GKASSERT(m_pConnection !=NULL);
	PGconn *conn=(PGconn*)m_pConnection;
	PQfinish(conn); 
	m_pConnection = NULL;

	return 0;
}

GKBASE::GKbool GKDatabasePG::SendQuery( const char* strQuery){ 
	GKASSERT(m_pConnection !=NULL);
	PGconn* pConn = (PGconn*)m_pConnection; 
	int retval = PQsendQuery(pConn, strQuery);
	if (retval == 0) {
		fprintf(stderr, "Send Query to database failed: %s",
			PQerrorMessage(pConn));
	}
	return retval != 0;
}

GKRecordset* GKDatabasePG::GetRecordset() { 
	GKASSERT(m_pConnection !=NULL);
	PGconn* pConn = (PGconn*)m_pConnection;
	PGresult* pResult= PQgetResult(pConn);

	GKRecordsetPG *pRecord = NULL;
	if (pResult != NULL) {
		pRecord = new GKRecordsetPG;
		pRecord->Create(pRecord);
	}

	return pRecord;
}

void GKDatabasePG::BeginTransaction() {
	GKASSERT(m_pConnection !=NULL);
	PGconn* pConn = (PGconn*)m_pConnection;
	PQexec(pConn, "begin transaction");
}

void GKDatabasePG::Commit() {
	GKASSERT(m_pConnection !=NULL);
	PGconn* pConn = (PGconn*)m_pConnection;
	PQexec(pConn, "commit"); 
}

NAMESPACEEND