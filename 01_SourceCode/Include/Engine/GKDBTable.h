/*
 *   ���ݿ����ʵ�ֵõ��࣬��ֱ�ӵ����ݿ��api�򽻵�
 *   ����װ�ǶԱ�ķ�װ�������Ĳ����������gis�����ݼ������ݿ����߼���û�й�ϵ��ȡ�պ������ֲ��������Ŀ��
 *   ������
 * 	 SetName() ���ñ�����
 * 	 AddColumns() ������� ���
 * 	 DelColumns() ɾ������ ���
 * 	 AddColumn()  ������� 
 * 	 DelColumn()  ɾ������
 * 	 AddRow()  ���Ԫ��
 * 	 DelRow()  ɾ��Ԫ��
 * 	 Modify()    �޸ı��е�����
 */

#include "Base/GKDataType.h"
#include "Base/GKString.h"

#ifndef GKDBTABLE_H_H
#define GKDBTABLE_H_H

NAMESPACEBEGIN(GKENGINE)

class ENGINE_API GKDBTable{
public:
	// ���ñ�����
	// param[in] �µı�����
	void SetName(GKString strName);

	// 
	// param[in]
	// param[in]
	// return
	
	
};

NAMESPACEEND
#endif