/*
 * brief: ���ݿ��Լ����, Լ����Ϊ��Լ�����м�Լ��
 * function list:
 * 
 * author: zhangkai
 * date: 2013��12��30��
 */
#ifndef GKDBCONSTRAINT_H_H
#define GKDBCONSTRAINT_H_H
#include "Base/GKDef.h"
#include "Base/GKString.h"
#include <vector>

NAMESPACEBEGIN(GKENGINE)

// �޸�: ��Լ����
// ����: 2013��11��28��
typedef enum{
	GKCONSNULL = 0,
	GKPRIMKEY = 0x00000001,  // ����Լ��
	GKNOTNULL = 0x00000002,  // �ǿ�Լ��
	GKUNIQUE = 0x00000004,   // ΨһԼ��
	GKCHECK = 0x00000008,    // ���Լ��
	GKDEFAULT=0x00000010,    // Ĭ��ֵԼ��
	GKFOREIGNTABLE=0x00000020 // ���Լ��
}GKConstraint;

// brief ��Լ��
class ENGINE_API GKTabConstraint{ 
public:
	// brief ��������
	// param[in] strPrimaryKey����
	void SetPrimaryKey(const GKBASE::GKString& IN strPrimaryKey);

	// brief �õ�����
	std::vector<GKBASE::GKString> GetPrimaryKeys() const;

	// brief �������
	void ClearPrimaryKeys();

	// brief ����ΨһԼ��
	// param[in] vCols ΨһԼ������
	void SetUnique(std::vector<GKBASE::GKString> vUniqueCols);

	// brief �õ�ΨһԼ��
	std::vector<std::vector<GKBASE::GKString> > GetUnique() const;

	// brief ���ΨһԼ��
	void ClearUnique();

	// brief ���Լ��
	void Clear();
private: 
	// ������ �����������������
	std::vector<GKBASE::GKString> m_vPrimaryKeys;
	// ΨһԼ����
	std::vector<std::vector<GKBASE::GKString> > m_vvUniques; 
};

// brief �м�Լ��
/*
	0x00000001 ��ʾ�ǿ�Լ��
	0x00000002 ��ʾĬ��ֵԼ��
	0x00000004 ��ʾcheckԼ��
	0x00000008 ��ʾ���Լ��
 */
class ENGINE_API GKColConstraint{
public:
	typedef enum{
		GKNOTNULL = 0x00000001,
		GKDEFVAL  = 0x00000002,
		GKCHECK   = 0x00000004,
		GKFORKEY  = 0x00000008
	}GKCons;
public:
	GKColConstraint();

	// brief ���÷ǿ�
	void SetNotNull();

	// brief ����Ĭ��ֵ
	// param[in] strDefault
	void SetDefault(const GKBASE::GKString& strDefault);

	// brief �õ�Ĭ��ֵ
	GKBASE::GKString GetDefault() const;

	// brief ����Check����
	// param[in] strCheck
	void SetCheck(const GKBASE::GKString& strCheck);

	// brief �õ�Check����
	GKBASE::GKString GetCheck() const;

	// brief �������
	// param[in] strForeignKey
	void SetForeignKey(const GKBASE::GKString& strForeignKey);

	// brief �õ����
	GKBASE::GKString GetForeignKey() const;

	// brief �ж��Ƿ�ǿ�
	GKBASE::GKbool IsNull() const;

	// brief �ж��Ƿ���CheckԼ��
	GKBASE::GKbool IsCheck() const;

	// brief �ж��Ƿ���Ĭ��ֵ
	GKBASE::GKbool IsDefaultValue() const;

	// brief �ж��Ƿ������
	GKBASE::GKbool IsForeignKey() const;

	// brief ���Լ������
	void Clear();
 private:	
	// brief Լ��ѡ��
	 GKBASE::GKuint32 m_uOptions;
	
	// brief Ĭ��ֵ
	GKBASE::GKString m_strDefValue;

	// brief Check����
	GKBASE::GKString m_strCheck;

	// brief ���
	GKBASE::GKString m_strForeignTable;
};

NAMESPACEEND
#endif