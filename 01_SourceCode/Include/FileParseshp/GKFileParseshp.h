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

struct _SHPHANDLE;
typedef struct _SHPHANDLE * SHPHandle;
struct _SHPobject;
typedef struct _SHPobject SHPObject;

struct _DBFinfo;
typedef struct _DBFinfo * DBFHandle;

NAMESPACEBEGIN(GKFILEPARSE)

class FILEPARSE_API GKFileParseshp{
public:
	enum shapetype{
		SHPT_NULL = 0,
		SHPT_POINT = 1,
		SHPT_ARC = 3,
		SHPT_POLYGON = 5,
		SHPT_MULTIPOINT = 8,
		SHPT_POINTZ = 11,
		SHPT_ARCZ = 13,
		SHPT_POLYGONZ = 15,
		SHPT_MULTIPOINTZ = 18,
		SHPT_POINTM = 21,
		SHPT_ARCM = 23,
		SHPT_POLYGONM = 25,
		SHPT_MULTIPOINTM = 28
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
	int Open(const char * strFilePath);
	void Close();

	void LoadInfo();
	int GetEntities();
	shapetype GetShapetype();
	double *GetMinBound();
	double *GetMaxBound();

	SHPObject *GetEntity(int nIndex); 
	void DestroyEntity(SHPObject * entity);
	static void PrintEntity(SHPObject * entity);
	static int GetEntityID(SHPObject * entity);

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
	int m_nEntities;
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