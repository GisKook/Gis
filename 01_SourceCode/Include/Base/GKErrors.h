/* 
	将所有的错误号定义在这都定义在该文件中
	错误号要体现工程,如引擎相关的错误第一个bit都是1
	EDSS---Engine-DataSource-Sqlite
*/
#ifndef SOURCECODE_INCLUDE_BASE_GKERRORS_H_H
#define SOURCECODE_INCLUDE_BASE_GKERRORS_H_H
#include "Base/GKDef.h"

#define GKSUCCESS                         0x00000000 // 正确
#define EDSS_FILEEXIST                    0xF0000001 // sqlite数据库文件存在无法创建
#define EDSS_OPENFAILE                    0xF0000002 // sqlite文件打开失败

#endif