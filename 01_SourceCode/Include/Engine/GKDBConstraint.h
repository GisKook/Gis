/*
 * brief: 数据库的约束类, 约束分为表级约束和列级约束
 * function list:
 * 
 * author: zhangkai
 * date: 2013年12月30日
 */
#ifndef GKDBCONSTRAINT_H_H
#define GKDBCONSTRAINT_H_H

NAMESPACEBEGIN(GKENGINE)

// 修改: 列约束类
// 日期: 2013年11月28日
typedef enum{
	GKCONSNULL = 0,
	GKPRIMKEY = 0x00000001,  // 主键约束
	GKNOTNULL = 0x00000002,  // 非空约束
	GKUNIQUE = 0x00000004,   // 唯一约束
	GKCHECK = 0x00000008,    // 检查约束
	GKDEFAULT=0x00000010,    // 默认值约束
	GKFOREIGNTABLE=0x00000020 // 外键约束
}GKConstraint;

// brief 表级约束
class ENGINE_API GKTabConstraint{ 
public:
	// brief 设置主键
	// param[in] strPrimaryKey主键
	void SetPrimaryKey(const GKString& IN strPrimaryKey);

	// brief 得到主键
	std::vector<GKString> GetPrimaryKeys() const;

	// brief 清空主键
	void ClearPrimaryKeys();

	// brief 设置唯一约束
	// param[in] vCols 唯一约束的列
	void SetUnique(std::vector<GKString> vUniqueCols);

	// brief 得到唯一约束
	std::vector<std::vector<GKString> > GetUnique() const;

	// brief 清空唯一约束
	void ClearUnique();

	// brief 清空约束
	void Clear();
private: 
	// 主键列 考虑联合主键的情况
	std::vector<GKString> m_vPrimaryKeys;
	// 唯一约束列
	std::vector<std::vector<GKString> > m_vvUniques; 
};

// brief 列级约束
/*
	0x00000001 表示非空约束
	0x00000002 表示默认值约束
	0x00000004 表示check约束
	0x00000008 表示外键约束
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

	// brief 设置非空
	void SetNotNull();

	// brief 设置默认值
	// param[in] strDefault
	void SetDefault(const GKString& strDefault);

	// brief 得到默认值
	GKString GetDefault() const;

	// brief 设置Check条件
	// param[in] strCheck
	void SetCheck(const GKString& strCheck);

	// brief 得到Check条件
	GKString GetCheck() const;

	// brief 设置外键
	// param[in] strForeignKey
	void SetForeignKey(const GKString& strForeignKey);

	// brief 得到外键
	GKString GetForeignKey() const;

	// brief 判断是否非空
	GKbool IsNull() const;

	// brief 判断是否有Check约束
	GKbool IsCheck() const;

	// brief 判断是否有默认值
	GKbool IsDefaultValue() const;

	// brief 判断是否有外键
	GKbool IsForeignKey() const;

	// brief 清空约束内容
	void Clear();
 private:	
	// brief 约束选项
	GKuint32 m_uOptions;
	
	// brief 默认值
	GKString m_strDefValue;

	// brief Check条件
	GKString m_strCheck;

	// brief 外键
	GKString m_strForeignTable;
};

NAMESPACEEND
#endif