/*
	本类的负责与数据库进行交互,保持DataSource,Dataset等的设计完整性,与数据库的交互就交给本类来实现吧~
	GKDBImp的所有接口要被继承.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H

namespace GKENGINE{

// brief 数据库类型
typedef enum{
	GKSQLITEDB=0,
	GKORACLE=1,
}GKDBType;

// 数据库连接信息
struct GKDBConnInfo
{
	GKBASE::GKString strServer;
	GKBASE::GKString strUser;
	GKBASE::GKString strPwd;
	GKDBType nDBType;
};

class ENGINE_API GKDatabase
{
	// 执行sql
	// param [in] 数据源
	// param [in] sql语句
	virtual GKBASE::GKErrorCode ExceSQL(GKDataSource*, const GKBASE::GKString&) = 0;

	// 连接数据库
	// nDataSourceConnInfo[in] 数据源连接信息
	virtual GKBASE::GKErrorCode Connect(const GKDBConnInfo& nDataSourceConnInfo) = 0;

	// 断开数据库连接
	virtual GKBASE::GKErrorCode DisConnect() = 0;

	// brief 得到数据库类型
	inline GKDBType GetType(){return m_nType;};

	// brief 创建表
	virtual GKBASE::GKErrorCode CreateTable() = 0; 

private:
	GKDatabase GKDatabase(const GKDBImp&);

private:
	// 数据库连接
	void* m_pConnection;

	// brief 数据库类型
	GKDBType m_nType;
};

}
#endif