/* 
	�����еĴ���Ŷ������ⶼ�����ڸ��ļ���
	�����Ҫ���ֹ���,��������صĴ����һ��bit����1
	EDSS---Engine-DataSource-Sqlite
*/
#ifndef SOURCECODE_INCLUDE_BASE_GKERRORS_H_H
#define SOURCECODE_INCLUDE_BASE_GKERRORS_H_H
#include "Base/GKDef.h"
#include "Base/GKDataType.h"

#define GKSUCCESS                         0x00000000 // success
#define E_FILEPATH_UNSPORT_I18N           0x00000001 // filepath donot support i18n

char * GKErrorStr(GKBASE::GKErrorCode errcode);

#endif