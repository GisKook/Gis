#include "FileParseshp/GKFileParseshp.h"
#include "shapefile.h"

using namespace FILEPARSE;

FILEPARSE::GKFileParseshp::GKFileParseshp():
m_shphandle(NULL),
m_nEntities(NULL),
m_nShapeType(NULL),
m_dMinBound(NULL),
m_dMaxBound(NULL)
{}

int FILEPARSE::GKFileParseshp::Open( const char * strFilePath )
{ 
	m_shphandle = SHPOpen(strFilePath, "rb");

	return m_shphandle == NULL;
}

void FILEPARSE::GKFileParseshp::Close()
{
	if(m_shphandle != NULL) {
		SHPClose(m_shphandle);
		m_shphandle = NULL;
	}
}

void FILEPARSE::GKFileParseshp::GetInfo()
{ 
	if(m_shphandle != NULL){
		SHPGetInfo(m_shphandle, m_nEntities, m_nShapeType, m_dMinBound, m_dMaxBound);
	}
}

FILEPARSE::GKFileParseshp::~GKFileParseshp()
{ 
	if (m_shphandle != NULL)
	{
		Close();
	}
}