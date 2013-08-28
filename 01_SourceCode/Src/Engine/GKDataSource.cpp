#include "Engine/GKDataSource.h"

using namespace GKBASE;
#define SUFFIX _U("_1")
namespace GKENGINE{

const GKDataset* GKENGINE::GKDataSource::GetDataset( const GKString& strDatsetName ) const
{
	std::map<GKBASE::GKString, GKDataset*>::const_iterator it = m_mapDatasets.find(strDatsetName);
	if (it != m_mapDatasets.end())
	{	
		return it->second;
	}

	return NULL;
}


const GKDataset* GKDataSource::GetDataset( GKuint32 nIndex ) const
{
	GKASSERT(nIndex < m_mapDatasets.size());
	if (nIndex < m_mapDatasets.size())
	{
		std::map<GKString, GKDataset*>::const_iterator it = m_mapDatasets.begin();
		for (GKuint32 i = 0; i < nIndex; nIndex++, it++);
		return it->second;
	}
	return NULL;
}

void GKDataSource::AddDataset( GKDataset* pDataset )
{
	GKASSERT(pDataset != NULL);
	if (pDataset != NULL)
	{
		GKString strDatasetName = pDataset->GetName();
		m_mapDatasets.insert(std::pair<GKString, GKDataset*>(strDatasetName, pDataset));
	}
}
GKBASE::GKString& GKDataSource::GetAvaliableName( GKBASE::GKString& strName )
{
	if (m_mapDatasets.find(strName) != NULL)
	{
		strName += SUFFIX;
		while(m_mapDatasets.find(strName) != NULL)
		{
			strName += SUFFIX;
		}
	}
	
	return strName;
}

GKBASE::GKuint32 GKDataSource::GetDatasetCount() const
{
	return m_mapDatasets.size();
}

}
