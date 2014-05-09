/*
 * brief: SQL query 出来的结果记录集
 * function list:
 * 
 * author: a staff of CETCNAV
 * date: 2014年5月7日
 */
#ifndef GKRECORDSET_H_H_H
#define GKRECORDSET_H_H_H

#include "Base/GKDataType.h"

NAMESPACEBEGIN(GKENGINE)

class ENGINEPG_API GKRecordset{
public:
	// brief 设置Recordset指针
	// param[in] pRecordset
	inline void Create(void* pRecord){m_pRecordset = pRecord;};

	// brief 清除Recordset内容
	virtual void Destroy()=0;

	// brief 得到属性个数
	virtual int GetFieldCount() = 0;

	// brief 得到元组的个数
	virtual int GetTupleCount() = 0; 

	// brief 得到指定的值
	virtual char* GetValue(int nTuple, int nField) = 0;
		

protected:
	void* m_pRecordset;
};

NAMESPACEEND

#endif