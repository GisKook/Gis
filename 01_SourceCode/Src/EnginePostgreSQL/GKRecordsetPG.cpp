#include "EnginePostgreSQL/GKRecordsetPG.h"
#include "libpq-fe.h"

using namespace GKENGINE;
using namespace GKBASE;

void GKRecordset::Destroy() {
	PGresult *pResult=(PGresult*)m_pRecordset;
	PQclear(pResult);
	m_pRecordset = NULL;
}

char* GKENGINE::GKRecordsetPG::GetValue( int nTuple, int nField ) { 
	PGresult *pResult=(PGresult*)m_pRecordset;
	return PQgetvalue(pResult, nTuple, nField);
}

void GKENGINE::GKRecordsetPG::Destroy() {
	PGresult *pResult=(PGresult*)m_pRecordset;
	PQclear(pResult);
}

int GKENGINE::GKRecordsetPG::GetFieldCount() {
	PGresult *pResult=(PGresult*)m_pRecordset;
	
	return PQnfields(pResult);
}

int GKENGINE::GKRecordsetPG::GetTupleCount() {
	PGresult *pResult=(PGresult*)m_pRecordset;

	return PQntuples(pResult);

}
