/*
      数据源基类，各种数据库应该派生自他，是在各个数据库之上的基类
	  数据源中要有一个系统表,用来存储一些表的信息,就叫GKREG表吧.这个表的字段包括
	  TID(GKuint32), DtName(GKString), TName(GKString), SRID(GKulong), Option(GKulong), Des(GKString)
	  数据源的名称必须是惟一的.
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
	// 得到数据源类型
	virtual GKDataSourceType GetDataSourceType() const = 0;

	// 创建数据集
	// param [in] 数据集信息
	virtual GKDataset* CreateDataset(const GKDatasetInfo* dtInfo) = 0;

	// 打开数据源
	// param[in] 数据库连接信息
	virtual GKBASE::GKErrorCode Open(const GKDBConnInfo& dbConnInfo) = 0;

	// 关闭数据源
	// param[in] 数据源名称
	virtual GKBASE::GKErrorCode Close(const GKBASE::GKString& strDsName ) = 0;

	// 关闭数据集
	// param[in] 数据源次序
	virtual GKBASE::GKErrorCode Close(GKBASE::GKuint32 nIndex) = 0
		
	// 删除数据集
	// param nIndex[in] 数据集次序
	virtual GKBASE::GKErrorCode DeleteDataset(GKBASE::GKuint32 nIndex) = 0;

	// 删除数据集
	// param strDatasetName[in] 数据集名称
	virtual GKBASE::GKErrorCode DeleteDataset(const GKBASE::GKString& strDatasetName) = 0;

	// 得到数据集
	// param strDatasetName[in] 数据集名称
	const GKDataset* GetDataset(const GKBASE::GKString& strDatsetName) const;

	// 得到数据集
	// param nIndex[in] 数据集的Index
	const GKDataset* GetDataset(GKBASE::GKuint32 nIndex) const;

	// 添加数据库
	// pDataset[in] 要添加的数据库
	// remark 调用这个方法是pDataset的名称应该是唯一的了.		
	void AddDataset(GKDataset* pDataset);

	// 得到数据集的个数
	GKBASE::GKuint32 GetDatasetCount() const;

	// 设置描述信息
	// strDescription[in] 设置描述信息
	GKBASE::GKErrorCode SetDescription(const GKBASE::GKString& strDescription){m_strDescription = strDescription;}

	// 得到描述信息
	GKBASE::GKString GetDescription() const {return m_strDescription;}

public:
	// 得到可以的数据集的名称
	// param strName[in] 名称
	GKBASE::GKString& GetAvaliableName(const GKBASE::GKString& strName);
	
private:
	// 数据集
	std::map< GKBASE::GKString, GKDataset* > m_mapDatasets;

	// 数据源描述信息
	GKBASE::GKString m_strDescription;

	// 数据库实现
	GKDBImp* m_pDBImp;
};

}

#endif