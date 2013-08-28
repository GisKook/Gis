/*
	����ĸ��������ݿ���н���,����DataSource,Dataset�ȵ����������,�����ݿ�Ľ����ͽ���������ʵ�ְ�~
	GKDBImp�����нӿ�Ҫ���̳�.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H

namespace GKENGINE{

class GKDataSource;

// ���ݿ�������Ϣ
struct GKDBConnInfo
{
	GKBASE::GKString strServer;
	GKBASE::GKString strUser;
	GKBASE::GKString strPwd;
};

class ENGINE_API GKDBImp
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

private:
	GKDBImp GKDBImp(const GKDBImp&);

private:

	// ���ݿ�����
	void* m_pConnection;
};


}
#endif