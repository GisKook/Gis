/*
 * brief: ���ݿ��Լ����, Լ����Ϊ��Լ�����м�Լ��
 * function list:
 * 
 * author: zhangkai
 * date: 2013��12��30��
 */
#ifndef GKDBCONSTRAINT_H_H
#define GKDBCONSTRAINT_H_H

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
	void SetPrimaryKey(const GKString& IN strPrimaryKey);

	// brief �õ�����
	std::vector<GKString> GetPrimaryKeys() const;

	// brief �������
	void ClearPrimaryKeys();

	// brief ����ΨһԼ��
	// param[in] vCols ΨһԼ������
	void SetUnique(std::vector<GKString> vUniqueCols);

	// brief �õ�ΨһԼ��
	std::vector<std::vector<GKString> > GetUnique() const;

	// brief ���ΨһԼ��
	void ClearUnique();

	// brief ���Լ��
	void Clear();
private: 
	// ������ �����������������
	std::vector<GKString> m_vPrimaryKeys;
	// ΨһԼ����
	std::vector<std::vector<GKString> > m_vvUniques; 
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
	void SetDefault(const GKString& strDefault);

	// brief �õ�Ĭ��ֵ
	GKString GetDefault() const;

	// brief ����Check����
	// param[in] strCheck
	void SetCheck(const GKString& strCheck);

	// brief �õ�Check����
	GKString GetCheck() const;

	// brief �������
	// param[in] strForeignKey
	void SetForeignKey(const GKString& strForeignKey);

	// brief �õ����
	GKString GetForeignKey() const;

	// brief �ж��Ƿ�ǿ�
	GKbool IsNull() const;

	// brief �ж��Ƿ���CheckԼ��
	GKbool IsCheck() const;

	// brief �ж��Ƿ���Ĭ��ֵ
	GKbool IsDefaultValue() const;

	// brief �ж��Ƿ������
	GKbool IsForeignKey() const;

	// brief ���Լ������
	void Clear();
 private:	
	// brief Լ��ѡ��
	GKuint32 m_uOptions;
	
	// brief Ĭ��ֵ
	GKString m_strDefValue;

	// brief Check����
	GKString m_strCheck;

	// brief ���
	GKString m_strForeignTable;
};

NAMESPACEEND
#endif