#include "Base/GKByteString.h"

namespace GKBASE{

extern BASE_API GKchar* g_pCharset[] = 
{
	"\0",
	"\0",
	"\0",
	"GB2312",
	"GBK",
	"GB18030",
	"BIG5", // ?
	"SHIFTJIS",
	"HANGEUL",
	"HANGUL",
	"OEM",
	"JOHAB",
	"HEBREW",
	"ARABIC",       
	"GREEK",
	"TURKISH",
	"VIETNAMESE",
	"THAI",
	"EASTEUROPE",
	"RUSSIAN",
	"MAC",
	"BALTIC",
	"UTF-8", // Unicode �ַ�����һ�ֱ��뷽ʽ
	"UTF16" // Unicode �ַ�����һ�ֱ��뷽ʽ
};

extern GKByteString operator+( const GKByteString& str1, const GKByteString& str2 )
{
	return GKByteString(str1.StdStr()+str2.StdStr());
}

extern GKbool operator==( const GKByteString& str1, const GKByteString& str2 )
{
	return str1.StdStr() == str2.StdStr();
}

extern GKbool operator!=( const GKByteString& str1, const GKByteString& str2 )
{
	return !(str1 == str2);
}



}

