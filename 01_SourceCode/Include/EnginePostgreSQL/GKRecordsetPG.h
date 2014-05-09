/*
 * brief: 查询返回的结果
 * function list:
 * 
 * author: a staff of CETCNAV
 * date: 2014年5月7日
 */
#ifndef GKRECORDSET_H_H
#define GKRECORDSET_H_H

#include "Engine/GKRecordset.h"

NAMESPACEBEGIN(GKENGINE)

class ENGINEPG_API GKRecordsetPG:public GKRecordset{
public:
	// brief 清除Recordset
	virtual void Destroy();

	// brief 得到属性个数
	virtual int GetFieldCount();

	// brief 得到元组的个数
	virtual int GetTupleCount(); 

	// brief 得到指定的值
	virtual char* GetValue(int nTuple, int nField);
		
};

NAMESPACEEND
#endif