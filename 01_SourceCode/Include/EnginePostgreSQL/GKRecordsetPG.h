/*
 * brief: ��ѯ���صĽ��
 * function list:
 * 
 * author: a staff of CETCNAV
 * date: 2014��5��7��
 */
#ifndef GKRECORDSET_H_H
#define GKRECORDSET_H_H

#include "Engine/GKRecordset.h"

NAMESPACEBEGIN(GKENGINE)

class ENGINEPG_API GKRecordsetPG:public GKRecordset{
public:
	// brief ���Recordset
	virtual void Destroy();

	// brief �õ����Ը���
	virtual int GetFieldCount();

	// brief �õ�Ԫ��ĸ���
	virtual int GetTupleCount(); 

	// brief �õ�ָ����ֵ
	virtual char* GetValue(int nTuple, int nField);
		
};

NAMESPACEEND
#endif