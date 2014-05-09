/*
 * brief: SQL query �����Ľ����¼��
 * function list:
 * 
 * author: a staff of CETCNAV
 * date: 2014��5��7��
 */
#ifndef GKRECORDSET_H_H_H
#define GKRECORDSET_H_H_H

#include "Base/GKDataType.h"

NAMESPACEBEGIN(GKENGINE)

class ENGINEPG_API GKRecordset{
public:
	// brief ����Recordsetָ��
	// param[in] pRecordset
	inline void Create(void* pRecord){m_pRecordset = pRecord;};

	// brief ���Recordset����
	virtual void Destroy()=0;

	// brief �õ����Ը���
	virtual int GetFieldCount() = 0;

	// brief �õ�Ԫ��ĸ���
	virtual int GetTupleCount() = 0; 

	// brief �õ�ָ����ֵ
	virtual char* GetValue(int nTuple, int nField) = 0;
		

protected:
	void* m_pRecordset;
};

NAMESPACEEND

#endif