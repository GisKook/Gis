#include <stdlib.h>
#include <string.h>
#include "FileParseshp/GKFileParseshp.h"
#include "shapefile.h"

using namespace GKFILEPARSE;

GKFILEPARSE::GKFileParseshp::GKFileParseshp():
m_shphandle(NULL),
m_nEntities(0),
m_nShapeType(0),
m_nFieldCount(0),
m_nRecordCount(0),
m_Fields(NULL)
{
	memset(m_dMinBound, 0, sizeof(double)*EXTREMUM);
	memset(m_dMaxBound, 0, sizeof(double)*EXTREMUM);
}

int GKFILEPARSE::GKFileParseshp::Open( const char * strFilePath )
{ 
	m_shphandle = SHPOpen(strFilePath, "rb");

	return m_shphandle == NULL;
}

void GKFILEPARSE::GKFileParseshp::Close()
{
	if(m_shphandle != NULL) {
		SHPClose(m_shphandle);
		m_shphandle = NULL;
	}
}

void GKFILEPARSE::GKFileParseshp::LoadInfo()
{ 
	if(m_shphandle != NULL){
		SHPGetInfo(m_shphandle, &m_nEntities, &m_nShapeType, m_dMinBound, m_dMaxBound);
	}
}

GKFILEPARSE::GKFileParseshp::~GKFileParseshp()
{ 
	if (m_shphandle != NULL)
	{
		Close();
	}
}

SHPObject * GKFILEPARSE::GKFileParseshp::GetEntity( int nIndex )
{ 
	return SHPReadObject(m_shphandle, nIndex);
}

void GKFILEPARSE::GKFileParseshp::DestroyEntity( SHPObject * entity )
{
	SHPDestroyObject(entity); 
	entity = NULL;
}

void GKFILEPARSE::GKFileParseshp::PrintEntity( SHPObject * entity )
{ 
    int		nSHPType = entity->nSHPType;
    int		nShapeId = entity->nShapeId;
    int		nParts = entity->nParts;
    int		*panPartStart = entity->panPartStart;
    int		*panPartType = entity->panPartType;
    
    int		nVertices = entity->nVertices;
    double	*padfX = entity->padfX;
    double	*padfY = entity->padfY;
    double	*padfZ = entity->padfZ;
    double	*padfM = entity->padfM;

    double	dfXMin = entity->dfXMin;
    double	dfYMin = entity->dfYMin;
    double	dfZMin = entity->dfZMin;
    double	dfMMin = entity->dfMMin;

    double	dfXMax = entity->dfXMax;
    double	dfYMax = entity->dfYMax;
    double	dfZMax = entity->dfZMax;
	double	dfMMax = entity->dfMMax;

    int		bMeasureIsUsed = entity->bMeasureIsUsed;

	fprintf(stdout, "nSHPType: %d nShapeID: %d nParts: %d ", nSHPType, nShapeId, nParts);
	int i;
	for(i = 0; i < nParts; ++i){
		fprintf(stdout, "panPartStart: %d panPartType: %d ", panPartStart[i], panPartType[i]);
	}
	
	fprintf(stdout, "nVertices: %d ", nVertices);
	for(i = 0; i < nVertices; ++i){
		fprintf(stdout, "padfX: %f pandfY: %f pandfZ: %f pandfM: %f ", padfX[i], padfY[i], padfZ[i], padfM[i]);
	}
	fprintf(stdout, "dfXMin: %f dfYMin: %f dfZMin: %f dfMMin: %f ", dfXMin, dfYMin, dfZMin, dfMMin);
	fprintf(stdout, "dfXMan: %f dfYMan: %f dfZMsn: %f dfMMsn: %f\n", dfXMax, dfYMax, dfZMax, dfMMax);
}

int GKFILEPARSE::GKFileParseshp::OpenDBF( const char * strFilePath )
{ 
	return (m_dbfhandle = DBFOpen(strFilePath, "rb")) == NULL;
}

void GKFILEPARSE::GKFileParseshp::CloseDBF()
{
	free(m_Fields);
	DBFClose(m_dbfhandle); 
}

void GKFILEPARSE::GKFileParseshp::LoadDBFInfo()
{
	m_nFieldCount = DBFGetFieldCount(m_dbfhandle); 
	m_nRecordCount = DBFGetRecordCount(m_dbfhandle);
	if(m_Fields != NULL){
		free(m_Fields);
		m_Fields = NULL;
	} 
	m_Fields = (struct FieldInfo *)malloc(sizeof(struct FieldInfo)*m_nFieldCount);
	if(m_Fields == NULL){
		fprintf(stderr, "malloc %d bytes error.%s %s %d\n", sizeof(struct FieldInfo) * m_nFieldCount, __FILE__, __FUNCTION__, __LINE__);
		return;
	}
	memset(m_Fields, 0, sizeof(struct FieldInfo) * m_nFieldCount); 
	int i = 0;
	int * pnWidth = NULL;
	int * pnDecimals = NULL;
	::DBFFieldType fieldtype = ::FTInvalid;
	for(i = 0; i < m_nFieldCount; ++i){ 
		fieldtype = DBFGetFieldInfo(m_dbfhandle, i, m_Fields[i].FieldName, pnWidth, pnDecimals);
		if (pnWidth != NULL) { 
			m_Fields[i].Width = *pnWidth;
		}else{
			m_Fields[i].Width = -1;
		}

		if(pnDecimals != NULL){
			m_Fields[i].Decimals = *pnDecimals;
		}else{
			m_Fields[i].Decimals = -1;
		}
		m_Fields[i].FieldType = (GKFileParseshp::DBFFieldType)fieldtype;
	}
}

int GKFILEPARSE::GKFileParseshp::GetFieldCount()
{
	return m_nFieldCount;
}

int GKFILEPARSE::GKFileParseshp::GetRecordCount()
{ 
	return m_nRecordCount;
}

int GKFILEPARSE::GKFileParseshp::ReadIntegerAttr( int nShapeID, int nField )
{
	return DBFReadIntegerAttribute(m_dbfhandle, nShapeID, nField); 
}

int GKFILEPARSE::GKFileParseshp::ReadDoubleAttr( int nShapeID, int nField )
{
	return DBFReadIntegerAttribute(m_dbfhandle, nShapeID, nField); 
}

const char * GKFILEPARSE::GKFileParseshp::ReadStringAttr( int nShapeID, int nField )
{
	return DBFReadStringAttribute(m_dbfhandle, nShapeID, nField); 
}

char * GKFILEPARSE::GKFileParseshp::GetFiledName( int nField )
{
	return (nField < m_nFieldCount)?m_Fields[nField].FieldName:NULL;
}

GKFileParseshp::DBFFieldType GKFILEPARSE::GKFileParseshp::GetFieldType( int nField )
{ 
	return (nField < m_nFieldCount)?m_Fields[nField].FieldType:GKFileParseshp::FTInvalid;
}

int GKFILEPARSE::GKFileParseshp::GetFieldWidth( int nField )
{ 
	return (nField < m_nFieldCount)?m_Fields[nField].Width:-1;
}

int GKFILEPARSE::GKFileParseshp::GetFieldDecimals( int nField )
{
	return (nField < m_nFieldCount) ? m_Fields[nField].Decimals : -1;
}

int GKFILEPARSE::GKFileParseshp::GetEntityID( SHPObject * entity )
{ 
	return entity->nShapeId;
}

int GKFileParseshp::GetEntities()
{
	return m_nEntities; 
}

GKFileParseshp::shapetype GKFileParseshp::GetShapetype()
{
	return (GKFileParseshp::shapetype)m_nShapeType;
}

double * GKFileParseshp::GetMinBound()
{ 
	return m_dMinBound;
}

double * GKFileParseshp::GetMaxBound()
{ 
	return m_dMaxBound;
}