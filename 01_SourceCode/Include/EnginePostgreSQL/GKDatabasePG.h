/*
 * brief: libpq.lib��wrapper,���ݿ⼶��Ĳ���
 * function list:
 * 
 * author: a staff of CETCNAV
 * date: 2014��5��7��
 */
#ifndef GKDATABASEPG_H_H
#define GKDATABASEPG_H_H
#include "Base/GKDef.h"
#include "Engine/GKDatabase.h"

NAMESPACEBEGIN(GKENGINE)

class ENGINEPG_API GKDatabasePG : public GKDatabase{
public:
	// brief �������ݿ�
	// param[in] ��������
	virtual GKBASE::GKErrorCode Connect(const GKDBConnInfo& dbConnInfo); 

	// �Ͽ����ݿ�����
	virtual GKBASE::GKErrorCode DisConnect(); 

	// brief ִ��Select���� �첽����
	// param[in] sql Ҫ��ִ�е�sql
	virtual GKBASE::GKbool SendQuery(const char* strQuery);

	// brief �ռ���SendQuery���͵�����Ľ��
	virtual GKRecordset* GetRecordset();

	// brief ��������
	void BeginTransaction();

	// brief ��������
	void Commit();
};

NAMESPACEEND

#endif