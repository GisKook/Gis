#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYMIXTURE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYMIXTURE_H_H
#include "Geometry/GKGeometry.h"

NAMESPACEBEGIN(GKGEOMETRY)

class GKGeometryMixture:public GKGeometry{

public:
	virtual GKBASE::GKuint8 GetType() const {return  (GKBASE::GKuint8)GKGeometry::GeoMixture;} 

	GKBASE::GKuint32 GetSubEntryCount() {return m_geos.size();}
	void AddEntry(GKGeometry * geos){m_geos.push_back(geos);}

private:
	std::vector<GKGeometry *>m_geos;

}

NAMESPACEEND
#endif