/*
	提供一些基础方法给其他工程用目前提供的方法有:
	GKFile::IsExist() // 判断文件或文件夹是否存在
	GKFile::Delete() // 删除文件或文件夹
	GKFile::MkDir() // 创建文件夹
*/
#ifndef SOURCECODE_INCLUDE_TOOLKIT_GKFILE_H_H
#define SOURCECODE_INCLUDE_TOOLKIT_GKFILE_H_H

#include "Base/GKString.h"

namespace GKBASE
{

class TOOLKIT_API GKFile
{
public:
	// 判断文件或路径是否存在
	// strPath[in] 文件路径或文件夹路径
	static GKBASE::GKbool IsExist(const GKBASE::StringPiece strPath);

	// 删除文件或文件夹
	// strPath[in] 文件路径或文件夹路径
	static GKBASE::GKbool Delete(const GKBASE::StringPiece strPath);

	// 创建文件夹
	// strPath[in] 文件路径或文件夹路径
	static GKBASE::GKbool MkDir(const GKBASE::StringPiece strPath);
};

}
#endif
