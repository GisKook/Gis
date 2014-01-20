/*
	����ĸ��������ݿ���н���,����DataSource,Dataset�ȵ����������,�����ݿ�Ľ����ͽ���������ʵ�ְ�~
	GKDBImp�����нӿ�Ҫ���̳�.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H

namespace GKENGINE{

// brief ���ݿ�����
typedef enum{
	GKSQLITEDB=0,
	GKORACLE=1,
}GKDBType;

// ���ݿ�������Ϣ
struct GKDBConnInfo
{
	GKBASE::GKString strServer;
	GKBASE::GKString strUser;
	GKBASE::GKString strPwd;
	GKDBType nDBType;
};

class ENGINE_API GKDatabase
{
	// ִ��sql
	// param [in] ����Դ
	// param [in] sql���
	virtual GKBASE::GKErrorCode ExceSQL(GKDataSource*, const GKBASE::GKString&) = 0;

	// �������ݿ�
	// nDataSourceConnInfo[in] ����Դ������Ϣ
	virtual GKBASE::GKErrorCode Connect(const GKDBConnInfo& nDataSourceConnInfo) = 0;

	// �Ͽ����ݿ�����
	virtual GKBASE::GKErrorCode DisConnect() = 0;

	// brief �õ����ݿ�����
	inline GKDBType GetType(){return m_nType;};

	// brief ������
	virtual GKBASE::GKErrorCode CreateTable() = 0; 

private:
	GKDatabase GKDatabase(const GKDBImp&);

private:
	// ���ݿ�����
	void* m_pConnection;

	// brief ���ݿ�����
	GKDBType m_nType;
};

}
#endif