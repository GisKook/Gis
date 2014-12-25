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

public:
	GKFileParseshp();
	~GKFileParseshp();
	int Open(const char * strFilePath);
	void Close();
	void GetInfo();
	int GetEntities();
	shapetype GetShapetype();
	double *GetMinBound();
	double *GetMaxBound();
	
private:
	enum{
		EXTREMUM = 4
	};
private:
	SHPHandle m_shphandle;
	int m_nEntities;
	int m_nShapeType;
	double m_dMinBound[EXTREMUM];
	double m_dMaxBound[EXTREMUM];
};

NAMESPACEEND

#endif