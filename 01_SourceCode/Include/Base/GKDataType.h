/*   
	brief: 对c++基本类型进行封装
	autor: zhangkai
	date:  2011-12-6	
*/
#ifndef SOURCECODE_INCLUDE_DATATYPE_H_H
#define SOURCECODE_INCLUDE_DATATYPE_H_H

#include "Base/GKDef.h"

NAMESPACEBEGIN(GKBASE)

typedef unsigned char			GKbyte;
typedef char					GKchar;
typedef float             		GKfloat;
typedef double            		GKdouble;
typedef bool              		GKbool;
typedef short             		GKshort;
typedef unsigned short    		GKushort;
typedef char			  		GKint8;
typedef unsigned char     		GKuint8;
typedef short		      		GKint16;
typedef unsigned short    		GKuint16;
typedef int               		GKint32;
typedef unsigned int      		GKuint32;
typedef long long int     		GKlong;
typedef unsigned long long int  GKulong;
typedef GKuint32                GKErrorCode;
//下面这个类型是从ICU中抄过来的。
/**
* \var GKuchar
* Define GKuchar to be wchar_t if that is 16 bits wide; always assumed to be unsigned.
* If wchar_t is not 16 bits wide, then define GKuchar to be uint16_t or char16_t because GCC >=4.4
* can handle UTF16 string literals.
* This makes the definition of GKuchar platform-dependent
* but allows direct string type compatibility with platforms with
* 16-bit wchar_t types.
*
* @draft ICU 4.4
*/

/* Define GKuchar to be compatible with wchar_t if possible. */
#if U_SIZEOF_WCHAR_T==2
typedef wchar_t GKuchar;
#elif U_GNUC_UTF16_STRING
#if defined _GCC_
typedef __CHAR16_TYPE__ GKuchar;
#endif
typedef char16_t GKuchar;
#else
typedef unsigned short GKuchar;
#endif


struct GKPoint
{
	GKint32 x;
	GKint32 y;
};

struct GKPoint2d
{
	GKdouble x;
	GKdouble y;
};

struct GKMultiPoint
{
	GKuint32 nPointCount;
	GKint32* pData;
};

struct GKMultiPoint2d
{
	GKuint32 nPointCount;
	GKdouble* pPointsData;	
};

struct GKLine
{
	GKPoint ptStart;
	GKPoint ptEnd;
};

struct GKLine2d
{
	GKPoint2d ptStart;
	GKPoint2d ptEnd;
};

struct GKPolyLine
{
	GKuint32 nLineCount;
	GKLine* pLinesData;
};

struct GKPolyLine2d
{
	GKuint32 nLines;
	GKLine2d* pLinesData;
};

struct GKPolyPolyLine
{
	GKuint32 nPoints;
	GKuint32* pLines;
	GKint32* pPolypolyLineData;
};

struct GKPolyPolyLine2d
{
	GKuint32 nPoints;
	GKuint32* pLines;
	GKdouble* pPolypolyLine2dData;
};

struct GKRegion
{
	GKuint32 nPoints;
	GKint32* pPointsData;
};

struct GKRegion2d
{
	GKuint32 nPoints;
	GKdouble* pPoint2dsData;
};

struct MultiRegion
{
	GKuint32 nPoints;
	GKuint32* pRegions;
	GKint32* pRegionsData;
};

struct MultiRegion2d
{
	GKuint32 nPoints;
	GKuint32* pRegions;
	GKint32* pRegion2dsData;
};

struct GKRect
{
	GKint32 nLeft;
	GKint32 nTop;
	GKint32 nRight;
	GKint32 nBottom;
};

struct GKRect2d
{
	GKdouble nLeft;
	GKdouble nTop;
	GKdouble nRight;
	GKdouble nBottom;
};

struct GKBounds2d
{
	GKdouble nLeft;
	GKdouble nTop;
	GKdouble nRight;
	GKdouble nBottom;
};

// 修改: 添加数据库操作相关的类型
// 日期: 2013年11月28日
typedef enum{
	GKINT=0,
	GKTEXT,
	GKDOUBLE,
	GKBLOB,
	GKNULL
}GKDBDatatype;
// 变体类型
typedef struct _GKVARIANT{
	GKDBDatatype eDataType; // 数据类型 0.int;1.text;2.double;3.blob
	typedef union{
		GKint32 iValue;
		GKdouble dValue;
		GKchar* pValue; 
		GKbyte* pData; 
	} GKVAR;
	int nCount; // 字符长度,内容长度
	GKVAR value;
#define iValue value.iValue
#define dValue value.dValue
#define pValue value.pValue
}GKVARIANT;

NAMESPACEEND

#endif
