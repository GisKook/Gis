// ����ϵͳ��ص��ö���װ������ļ���
#ifndef SOURCECODE_INCLUDE_BASE_SYSTEM_H_H
#define SOURCECODE_INCLUDE_BASE_SYSTEM_H_H
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "GKDef.h"
#include "GKExport.h"


// brief �õ��߾���ʱ�� 
// brief ��ȡʱ�������

class BASE_API GKTimer{
public:
	// brief ��ʼ��
	GKTimer();

	// brief ��ʼ
	void Start();

	// brief ����
	unsigned long long Stop(); 
public:
	unsigned long long m_uOverhead;

private:
	unsigned long long m_uStartCycle;
};

namespace GKBASE{
	GKBASE::GKCharset GetSystemDefaultLocal();
}

#endif // SOURCECODE_INCLUDE_BASE_SYSTEM_H_H