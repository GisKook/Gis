/*
	本类的负责与数据库进行交互,保持DataSource,Dataset等的设计完整性,与数据库的交互就交给本类来实现吧~
	GKDBImp的所有接口要被继承.
*/
#ifndef SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H
#define SOURCECODE_INCLUDE_ENGINE_GKENGINETOOLKIT_H_H

namespace GKENGINE{

class GKDataSource;

// 数据库连接信息
struct GKDBConnInfo
{
	GKBASE::GKString strServer;
	GKBASE::GKString strUser;
	GKBASE::GKString strPwd;
};

class ENGINE_API GKDBImp
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

private:
	GKDBImp GKDBImp(const GKDBImp&);

private:

	// 数据库连接
	void* m_pConnection;
};


}
#endif