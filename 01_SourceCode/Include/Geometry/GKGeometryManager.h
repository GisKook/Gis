// �������Geometry�Ĵ���,
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H
#include "Geometry/GKGeometry.h"

class GEOMETRY_API GKGeometryManager
{
public:
	// ���𴴽�Geometry
	// nGeoType[in] Ҫ���ɵ�Geometry����
	GKGeometry* CreateGeometry(GKGeometry::GeoType nGeoType);

};
#endif