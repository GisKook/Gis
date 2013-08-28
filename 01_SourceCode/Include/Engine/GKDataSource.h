/*
      ����Դ���࣬�������ݿ�Ӧ���������������ڸ������ݿ�֮�ϵĻ���
	  ����Դ��Ҫ��һ��ϵͳ��,�����洢һЩ�����Ϣ,�ͽ�GKREG���.�������ֶΰ���
	  TID(GKuint32), DtName(GKString), TName(GKString), SRID(GKulong), Option(GKulong), Des(GKString)
	  ����Դ�����Ʊ�����Ωһ��.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKDATASOURCE_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKDATASOURCE_H_H

#include <map>
#include <vector>
#include "Base/gkexports.h"
#include "Base/gkdefs.h"
#include "Base/DataType.h"
#include "Base/GKString.h"
#include "Engine/GKDataset.h"
#include "Engine/GKDBImp.h"

namespace GKENGINE
{
enum GKDataSourceType{
	SQLITEDB = 0
};

typedef union{
	struct GKDatasetInfo;
	GKBASE::GKuint32 nType;
} GKDatasetInfo;

class ENGINE_API GKDataSource
{

public:
	// �õ�����Դ����
	virtual GKDataSourceType GetDataSourceType() const = 0;

	// �������ݼ�
	// param [in] ���ݼ���Ϣ
	virtual GKDataset* CreateDataset(const GKDatasetInfo* dtInfo) = 0;

	// ������Դ
	// param[in] ���ݿ�������Ϣ
	virtual GKBASE::GKErrorCode Open(const GKDBConnInfo& dbConnInfo) = 0;

	// �ر�����Դ
	// param[in] ����Դ����
	virtual GKBASE::GKErrorCode Close(const GKBASE::GKString& strDsName ) = 0;

	// �ر����ݼ�
	// param[in] ����Դ����
	virtual GKBASE::GKErrorCode Close(GKBASE::GKuint32 nIndex) = 0
		
	// ɾ�����ݼ�
	// param nIndex[in] ���ݼ�����
	virtual GKBASE::GKErrorCode DeleteDataset(GKBASE::GKuint32 nIndex) = 0;

	// ɾ�����ݼ�
	// param strDatasetName[in] ���ݼ�����
	virtual GKBASE::GKErrorCode DeleteDataset(const GKBASE::GKString& strDatasetName) = 0;

	// �õ����ݼ�
	// param strDatasetName[in] ���ݼ�����
	const GKDataset* GetDataset(const GKBASE::GKString& strDatsetName) const;

	// �õ����ݼ�
	// param nIndex[in] ���ݼ���Index
	const GKDataset* GetDataset(GKBASE::GKuint32 nIndex) const;

	// ������ݿ�
	// pDataset[in] Ҫ��ӵ����ݿ�
	// remark �������������pDataset������Ӧ����Ψһ����.		
	void AddDataset(GKDataset* pDataset);

	// �õ����ݼ��ĸ���
	GKBASE::GKuint32 GetDatasetCount() const;

	// ����������Ϣ
	// strDescription[in] ����������Ϣ
	GKBASE::GKErrorCode SetDescription(const GKBASE::GKString& strDescription){m_strDescription = strDescription;}

	// �õ�������Ϣ
	GKBASE::GKString GetDescription() const {return m_strDescription;}

public:
	// �õ����Ե����ݼ�������
	// param strName[in] ����
	GKBASE::GKString& GetAvaliableName(const GKBASE::GKString& strName);
	
private:
	// ���ݼ�
	std::map< GKBASE::GKString, GKDataset* > m_mapDatasets;

	// ����Դ������Ϣ
	GKBASE::GKString m_strDescription;

	// ���ݿ�ʵ��
	GKDBImp* m_pDBImp;
};

}

#endif