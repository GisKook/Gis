/*
	����ĸ��������ݿ���н���,����DataSource,Dataset�ȵ����������,�����ݿ�Ľ����ͽ���������ʵ�ְ�~
	GKDBImp�����нӿ�Ҫ���̳�.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H
#include "Base/GKDataType.h"
#include "Base/GKErrors.h"
#include "Base/GKString.h"
#include "Engine/GKRecordset.h"

NAMESPACEBEGIN(GKENGINE)


// brief ���ݿ�����
typedef enum{
	GKSQLITEDB=0,
	GKPOSTGRESQL=1,
}GKDBType;

// ���ݿ�������Ϣ
struct GKPGConnInfo{
	const char* pghost;
	const char* pgport;
	const char* pgoptions;
	const char* pgtty;
	const char* dbName;
	const char* login;
	const char* passwd; 
	GKDBType type;
};

typedef union{
	struct GKPGConnInfo pgConn;
}GKDBConnInfo;

class ENGINE_API GKDatabase
{
public:
	// �������ݿ�
	// nDataSourceConnInfo[in] ����Դ������Ϣ
	virtual GKBASE::GKErrorCode Connect(const GKDBConnInfo& nDataSourceConnInfo) = 0;

	// �Ͽ����ݿ�����
	virtual GKBASE::GKErrorCode DisConnect() = 0;

	// brief �õ����ݿ�����
	inline GKDBType GetType();

	// ִ��sql
	// param [in] sql���
	virtual GKBASE::GKErrorCode ExceSQL(const GKBASE::GKString&) = 0;

	// brief ִ��Select���� �첽����
	// param[in] sql Ҫ��ִ�е�sql
	virtual GKBASE::GKbool SendQuery(const char* ) = 0;

	// brief �ռ���SendQuery���͵�����Ľ��
	virtual GKRecordset* GetRecordset() = 0;

protected:
	// ���ݿ�����
	void* m_pConnection;

	// brief ���ݿ�����
	GKDBType m_nType;
};

NAMESPACEEND
#endif