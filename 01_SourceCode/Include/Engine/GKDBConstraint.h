/*
 * brief: 数据库的约束类, 约束分为表级约束和列级约束
 * function list:
 * 
 * author: zhangkai
 * date: 2013年12月30日
 */
#ifndef GKDBCONSTRAINT_H_H
#define GKDBCONSTRAINT_H_H
#include "Base/GKDef.h"
#include "Base/GKString.h"
#include <vector>

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
	void SetPrimaryKey(const GKBASE::GKString& IN strPrimaryKey);

	// brief 得到主键
	std::vector<GKBASE::GKString> GetPrimaryKeys() const;

	// brief 清空主键
	void ClearPrimaryKeys();

	// brief 设置唯一约束
	// param[in] vCols 唯一约束的列
	void SetUnique(std::vector<GKBASE::GKString> vUniqueCols);

	// brief 得到唯一约束
	std::vector<std::vector<GKBASE::GKString> > GetUnique() const;

	// brief 清空唯一约束
	void ClearUnique();

	// brief 清空约束
	void Clear();
private: 
	// 主键列 考虑联合主键的情况
	std::vector<GKBASE::GKString> m_vPrimaryKeys;
	// 唯一约束列
	std::vector<std::vector<GKBASE::GKString> > m_vvUniques; 
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
	void SetDefault(const GKBASE::GKString& strDefault);

	// brief 得到默认值
	GKBASE::GKString GetDefault() const;

	// brief 设置Check条件
	// param[in] strCheck
	void SetCheck(const GKBASE::GKString& strCheck);

	// brief 得到Check条件
	GKBASE::GKString GetCheck() const;

	// brief 设置外键
	// param[in] strForeignKey
	void SetForeignKey(const GKBASE::GKString& strForeignKey);

	// brief 得到外键
	GKBASE::GKString GetForeignKey() const;

	// brief 判断是否非空
	GKBASE::GKbool IsNull() const;

	// brief 判断是否有Check约束
	GKBASE::GKbool IsCheck() const;

	// brief 判断是否有默认值
	GKBASE::GKbool IsDefaultValue() const;

	// brief 判断是否有外键
	GKBASE::GKbool IsForeignKey() const;

	// brief 清空约束内容
	void Clear();
 private:	
	// brief 约束选项
	 GKBASE::GKuint32 m_uOptions;
	
	// brief 默认值
	GKBASE::GKString m_strDefValue;

	// brief Check条件
	GKBASE::GKString m_strCheck;

	// brief 外键
	GKBASE::GKString m_strForeignTable;
};

NAMESPACEEND
#endif