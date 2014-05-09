/*
 * brief: libpq.lib的wrapper,数据库级别的操作
 * function list:
 * 
 * author: a staff of CETCNAV
 * date: 2014年5月7日
 */
#ifndef GKDATABASEPG_H_H
#define GKDATABASEPG_H_H
#include "Base/GKDef.h"
#include "Engine/GKDatabase.h"

NAMESPACEBEGIN(GKENGINE)

class ENGINEPG_API GKDatabasePG : public GKDatabase{
public:
	// brief 连接数据库
	// param[in] 连接内容
	virtual GKBASE::GKErrorCode Connect(const GKDBConnInfo& dbConnInfo); 

	// 断开数据库连接
	virtual GKBASE::GKErrorCode DisConnect(); 

	// brief 执行Select操作 异步操作
	// param[in] sql 要被执行的sql
	virtual GKBASE::GKbool SendQuery(const char* strQuery);

	// brief 收集由SendQuery发送的命令的结果
	virtual GKRecordset* GetRecordset();

	// brief 开启事务
	void BeginTransaction();

	// brief 结束事务
	void Commit();
};

NAMESPACEEND

#endif