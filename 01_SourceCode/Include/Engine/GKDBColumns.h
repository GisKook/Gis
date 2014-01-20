/*
 * ����: ��Ӧ���ݿ�����ԣ���Ϊ������:GKDBColumns/GKDBColumn/GKConstraint
 * �����б�:
 * 
 * ����: zhangkai
 * ����: 2013��11��28��
 */
#ifndef GKDBCOLUMNS_H_H
#define GKDBCOLUMNS_H_H
#include <vector>
#include "Base/GKDef.h" 
#include "Base/GKDataType.h"
NAMESPACEBEGIN(GKENGINE) 

// ����
class ENGINE_API GKDBColumn{
 public:
	

public:
	GKDBColumn();
	// ��������
	// param eType[in] ����
	int SetType(GKDatatype IN eType);

	// ����Լ��
	// param eCons[in] Լ��
	int SetConstraint(int IN nCons);

	// ����Ĭ��ֵ
	// param strValue[in] Ĭ��ֵ
	int SetDefaultValue(CNVARIANT IN value);

	// ��������
	// param strName[in] ����
	int SetName(GKString IN strName);
	
	// �õ�����
	GKString GetName() const;

	// ����
	int SetEmpty();
public:
	// ������
	std::string m_strName;

	// �е�����
	std::string m_strType;

	// �е�Լ��
	std::string m_strConstraint;

	// Ĭ��ֵ
	std::string m_strDefalutValue;
public:

	int m_nCons;
};

class ENGINE_API CNDBColumns{
public:
	CNDBColumns();
public:
	// ���
	// param Col[in] ��
	int Add(const CNDBColumn& IN Col);

	// ���
	// param strName [in] �ֶ�����
	// param eType [in] ����
	// param eCons [in] Լ��
	// param strDefaultValue[in] Ĭ��ֵ
	int Add(std::string strName, CNDBColumn::CNCOLTYPE eType, 
		CNDBColumn::CNCOLCONS eCons = CNDBColumn::CONSNULL, std::string strDefaultValue = "");

	// �����ɾ��
	// param nIndex[in]���
	int Delete(int IN nIndex);

	// ������ɾ��
	// param strName[in] ����
	int Delete(std::string IN strName);

	// �õ�ָ������
	// param nIndex[in] ���к�
	const CNDBColumn& GetAt(int IN nIndex) const;

	// �õ�Col������
	int GetSize() const;

	// �õ���һ��Ԫ��
	pos GetBegin() ;

	// �õ����һ��Ԫ��
	pos GetEnd() ;

private:
	// �洢�еļ���
	std::vector<CNDBColumn>m_vCols;
};

NAMESPACEEND
#endif 