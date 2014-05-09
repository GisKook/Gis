/*
	本类的负责与数据库进行交互,保持DataSource,Dataset等的设计完整性,与数据库的交互就交给本类来实现吧~
	GKDBImp的所有接口要被继承.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H
#include "Base/GKDataType.h"
#include "Base/GKErrors.h"
#include "Base/GKString.h"
#include "Engine/GKRecordset.h"

NAMESPACEBEGIN(GKENGINE)


// brief 数据库类型
typedef enum{
	GKSQLITEDB=0,
	GKPOSTGRESQL=1,
}GKDBType;

// 数据库连接信息
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
	// 连接数据库
	// nDataSourceConnInfo[in] 数据源连接信息
	virtual GKBASE::GKErrorCode Connect(const GKDBConnInfo& nDataSourceConnInfo) = 0;

	// 断开数据库连接
	virtual GKBASE::GKErrorCode DisConnect() = 0;

	// brief 得到数据库类型
	inline GKDBType GetType();

	// 执行sql
	// param [in] sql语句
	virtual GKBASE::GKErrorCode ExceSQL(const GKBASE::GKString&) = 0;

	// brief 执行Select操作 异步操作
	// param[in] sql 要被执行的sql
	virtual GKBASE::GKbool SendQuery(const char* ) = 0;

	// brief 收集由SendQuery发送的命令的结果
	virtual GKRecordset* GetRecordset() = 0;

protected:
	// 数据库连接
	void* m_pConnection;

	// brief 数据库类型
	GKDBType m_nType;
};

NAMESPACEEND
#endif