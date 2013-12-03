/*
	开源数据库SQLITE引擎.数据源.作为最通用的数据源进行存储
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
	// 得到引擎类型
	virtual GKEngineType GetEngineType() const {return GKENGINE::SQLITEDB;}
	
	// 连接引擎
	// nDataSourceConnInfo[in] 数据库连接信息
	virtual GKBASE::GKErrorCode Connect(const GKDataSourceConnInfo& nDataSourceConnInfo);

	// 断开数据库连接
	virtual GKBASE::GKErrorCode DisConnect() const {return 0;};

	// 创建数据集
	// param pDatasetInfo[in] 数据集信息
	virtual GKDataset* CreateDataset(GKDatasetInfo* pDatasetInfo);
	
	

private:
	// 加载
	virtual GKBASE::GKErrorCode LoadInfos() const {return 0;};
	
public:
	GKDataSourceSqlite();
	virtual ~GKDataSourceSqlite();

public:
	// 创建数据库
	// strPath[in] 数据库文件路径
	GKBASE::GKErrorCode Create(const GKBASE::GKString& strPath, GKBASE::GKbool bOverWrite = TRUE);

	// 得到连接
	sqlite3* GetConnection() const;

private:
	//! \brief 打开sqlite数据库
	//! \param filename 文件名
	//! \param flags 文件操作的几种方式ro rw rwc
	GKBASE::GKErrorCode Open( const GKBASE::GKString& strPath, GKBASE::GKuint32 nFlags);

	//! \brief 关闭sqlite数据库
	//! \param filename 文件名
	GKBASE::GKErrorCode Close() const; 

	//! \brief 删除sqlite数据库
	//! \param filename 文件名
	GKBASE::GKErrorCode Delete(const GKBASE::GKString& strFileName) const;

private:
	// 数据库连接
	sqlite3* m_pHandle; 
};

}
#endif
