#include <stdlib.h>
#include <string.h>
#include "FileParseshp/GKFileParseshp.h"
#include "shapefile.h"

using namespace GKFILEPARSE;

GKFILEPARSE::GKFileParseshp::GKFileParseshp():
m_shphandle(NULL),
m_nEntities(0),
m_nShapeType(0)
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

void GKFILEPARSE::GKFileParseshp::GetInfo()
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