/*
	��Դ���ݿ�SQLITE����.����Դ.��Ϊ��ͨ�õ�����Դ���д洢
*/
#ifndef SOURCECODE_INCLUDE_ENGINESQLITE_GKDATASOURCE_H_H
#define SOURCECODE_INCLUDE_ENGINESQLITE_GKDATASOURCE_H_H

#include "Base/gkexports.h"
#include "Engine/GKDataSource.h"

typedef struct sqlite3 sqlite3;

namespace GKENGINE 
{

class ENGINESQLITEDB_API GKDataSourceSqlite : public GKDataSource
{

public:
	enum
	{
		OPEN_READONLY =0x00000001,
		OPEN_READWRITE=0x00000002,
		OPEN_CREATE=0x00000004
	};	
	
public:
	// �õ���������
	virtual GKEngineType GetEngineType() const {return GKENGINE::SQLITEDB;}
	
	// ��������
	// nDataSourceConnInfo[in] ���ݿ�������Ϣ
	virtual GKBASE::GKErrorCode Connect(const GKDataSourceConnInfo& nDataSourceConnInfo);

	// �Ͽ����ݿ�����
	virtual GKBASE::GKErrorCode DisConnect() const {return 0;};

	// �������ݼ�
	// param pDatasetInfo[in] ���ݼ���Ϣ
	virtual GKDataset* CreateDataset(GKDatasetInfo* pDatasetInfo);
	
	

private:
	// ����
	virtual GKBASE::GKErrorCode LoadInfos() const {return 0;};
	
public:
	GKDataSourceSqlite();
	virtual ~GKDataSourceSqlite();

public:
	// �������ݿ�
	// strPath[in] ���ݿ��ļ�·��
	GKBASE::GKErrorCode Create(const GKBASE::GKString& strPath, GKBASE::GKbool bOverWrite = TRUE);

	// �õ�����
	sqlite3* GetConnection() const;

private:
	//! \brief ��sqlite���ݿ�
	//! \param filename �ļ���
	//! \param flags �ļ������ļ��ַ�ʽro rw rwc
	GKBASE::GKErrorCode Open( const GKBASE::GKString& strPath, GKBASE::GKuint32 nFlags);

	//! \brief �ر�sqlite���ݿ�
	//! \param filename �ļ���
	GKBASE::GKErrorCode Close() const; 

	//! \brief ɾ��sqlite���ݿ�
	//! \param filename �ļ���
	GKBASE::GKErrorCode Delete(const GKBASE::GKString& strFileName) const;

private:
	// ���ݿ�����
	sqlite3* m_pHandle; 
};

}
#endif
