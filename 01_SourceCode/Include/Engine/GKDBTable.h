/*
 *   数据库具体实现得到类，与直接的数据库的api打交道
 *   这层封装是对表的封装，定义表的操作，这个和gis的数据集、数据库在逻辑上没有关系争取日后可以移植到其他项目中
 *   函数：
 * 	 SetName() 设置表名称
 * 	 AddColumns() 添加属性 多个
 * 	 DelColumns() 删除属性 多个
 * 	 AddColumn()  添加属性 
 * 	 DelColumn()  删除属性
 * 	 AddRow()  添加元组
 * 	 DelRow()  删除元组
 * 	 Modify()    修改表中的内容
 */

#include "Base/GKDataType.h"
#include "Base/GKString.h"

#ifndef GKDBTABLE_H_H
#define GKDBTABLE_H_H

NAMESPACEBEGIN(GKENGINE)

class ENGINE_API GKDBTable{
public:
	// 设置表名称
	// param[in] 新的表名称
	void SetName(GKString strName);

	// 
	// param[in]
	// param[in]
	// return
	
	
};

NAMESPACEEND
#endif