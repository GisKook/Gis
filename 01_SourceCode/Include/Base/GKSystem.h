// 操作系统相关调用都封装在这个文件里
#ifndef SOURCECODE_INCLUDE_BASE_SYSTEM_H_H
#define SOURCECODE_INCLUDE_BASE_SYSTEM_H_H
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "GKDef.h"
#include "GKExport.h"


// brief 得到高精度时间 
// brief 读取时间计数器

class BASE_API GKTimer{
public:
	// brief 初始化
	GKTimer();

	// brief 开始
	void Start();

	// brief 结束
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