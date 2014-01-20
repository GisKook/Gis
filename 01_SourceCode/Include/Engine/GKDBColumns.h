/*
 * 功能: 对应数据库的属性，分为三个类:GKDBColumns/GKDBColumn/GKConstraint
 * 函数列表:
 * 
 * 作者: zhangkai
 * 日期: 2013年11月28日
 */
#ifndef GKDBCOLUMNS_H_H
#define GKDBCOLUMNS_H_H
#include <vector>
#include "Base/GKDef.h" 
#include "Base/GKDataType.h"
NAMESPACEBEGIN(GKENGINE) 

// 列类
class ENGINE_API GKDBColumn{
 public:
	

public:
	GKDBColumn();
	// 设置类型
	// param eType[in] 类型
	int SetType(GKDatatype IN eType);

	// 设置约束
	// param eCons[in] 约束
	int SetConstraint(int IN nCons);

	// 设置默认值
	// param strValue[in] 默认值
	int SetDefaultValue(CNVARIANT IN value);

	// 设置名称
	// param strName[in] 名称
	int SetName(GKString IN strName);
	
	// 得到名称
	GKString GetName() const;

	// 归零
	int SetEmpty();
public:
	// 列名称
	std::string m_strName;

	// 列的类型
	std::string m_strType;

	// 列的约束
	std::string m_strConstraint;

	// 默认值
	std::string m_strDefalutValue;
public:

	int m_nCons;
};

class ENGINE_API CNDBColumns{
public:
	CNDBColumns();
public:
	// 添加
	// param Col[in] 列
	int Add(const CNDBColumn& IN Col);

	// 添加
	// param strName [in] 字段名称
	// param eType [in] 类型
	// param eCons [in] 约束
	// param strDefaultValue[in] 默认值
	int Add(std::string strName, CNDBColumn::CNCOLTYPE eType, 
		CNDBColumn::CNCOLCONS eCons = CNDBColumn::CONSNULL, std::string strDefaultValue = "");

	// 按序号删除
	// param nIndex[in]序号
	int Delete(int IN nIndex);

	// 按名称删除
	// param strName[in] 名称
	int Delete(std::string IN strName);

	// 得到指定的列
	// param nIndex[in] 序列号
	const CNDBColumn& GetAt(int IN nIndex) const;

	// 得到Col的数量
	int GetSize() const;

	// 得到第一个元素
	pos GetBegin() ;

	// 得到最后一个元素
	pos GetEnd() ;

private:
	// 存储列的集合
	std::vector<CNDBColumn>m_vCols;
};

NAMESPACEEND
#endif 