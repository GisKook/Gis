/*
 * brief: shp文件转换
 * function list:
 * 
 * author: zhangkai
 * date: 2014年11月18日
 */
#ifndef FILEPARSE_FILEPARSESHP_H_H
#define FILEPARSE_FILEPARSESHP_H_H
#include "Base/GKDef.h"
#include "Base/GKExport.h"
#include "Base/GKString.h"
#include "Geometry/GKGeometry.h"

struct _SHPHANDLE;
typedef struct _SHPHANDLE * SHPHandle;
//struct _SHPobject;
//typedef struct _SHPobject SHPObject; 
struct _DBFinfo;
typedef struct _DBFinfo * DBFHandle;

NAMESPACEBEGIN(GKFILEPARSE)

class FILEPARSE_API GKFileParseshp{
public:
	enum shapetype{
		SHP_NULL = 0,

		SHP_POINT = 1,
		SHP_ARC = 3,
		SHP_POLYGON = 5,
		SHP_MULTIPOINT = 8,

		SHP_POINTZ = 11,
		SHP_ARCZ = 13,
		SHP_POLYGONZ = 15,
		SHP_MULTIPOINTZ = 18,

		SHP_POINTM = 21,
		SHP_ARCM = 23,
		SHP_POLYGONM = 25,
		SHP_MULTIPOINTM = 28
	};

	typedef enum {
	  FTString,
	  FTInteger,
	  FTDouble,
	  FTLogical,
	  FTInvalid
	} DBFFieldType;

public:
	GKFileParseshp();
	~GKFileParseshp();
public:
	GKGEOMETRY::GKGeometry * GetGeomerty(int index);

public:
	GKBASE::GKErrorCode Open(GKBASE::GKString filepath);
public:
	int Open_(const char * strFilePath);
	void Close();

	void LoadInfo();
	int GetEntitieCount();
	shapetype GetShapetype();
	double *GetMinBound();
	double *GetMaxBound();

public:
	int OpenDBF(const char * strFilePath);
	void CloseDBF();
	void LoadDBFInfo(); 
	int GetFieldCount();
	int GetRecordCount();
	char * GetFiledName(int nField);
	DBFFieldType GetFieldType(int nField);
	int GetFieldWidth(int nField);
	int GetFieldDecimals(int nField);

	int ReadIntegerAttr(int nShapeID, int nField);
	int ReadDoubleAttr(int nShapeID, int nField);
	const char * ReadStringAttr(int nShapeID, int nField);
	
private:
	enum{
		EXTREMUM = 4,
		MAXFIELDNAMELENGTH = 12 //  a dummy character for '\0'
	};
private:
	SHPHandle m_shphandle;
	int m_nEntitieCount;
	int m_nShapeType;
	double m_dMinBound[EXTREMUM];
	double m_dMaxBound[EXTREMUM];
	
private:
	struct FieldInfo{
		char FieldName[MAXFIELDNAMELENGTH];
		int  Width;
		int  Decimals;
		DBFFieldType FieldType;
	};
	DBFHandle m_dbfhandle;
	int m_nFieldCount;
	int m_nRecordCount;
	struct FieldInfo * m_Fields;
};

NAMESPACEEND

#endif