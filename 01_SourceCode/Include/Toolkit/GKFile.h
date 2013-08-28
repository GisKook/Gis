/*
	�ṩһЩ��������������������Ŀǰ�ṩ�ķ�����:
	GKFile::IsExist() // �ж��ļ����ļ����Ƿ����
	GKFile::Delete() // ɾ���ļ����ļ���
	GKFile::MkDir() // �����ļ���
*/
#ifndef SOURCECODE_INCLUDE_TOOLKIT_GKFILE_H_H
#define SOURCECODE_INCLUDE_TOOLKIT_GKFILE_H_H

#include "Base/GKString.h"

namespace GKBASE
{

class TOOLKIT_API GKFile
{
public:
	// �ж��ļ���·���Ƿ����
	// strPath[in] �ļ�·�����ļ���·��
	static GKBASE::GKbool IsExist(const GKBASE::StringPiece strPath);

	// ɾ���ļ����ļ���
	// strPath[in] �ļ�·�����ļ���·��
	static GKBASE::GKbool Delete(const GKBASE::StringPiece strPath);

	// �����ļ���
	// strPath[in] �ļ�·�����ļ���·��
	static GKBASE::GKbool MkDir(const GKBASE::StringPiece strPath);
};

}
#endif
