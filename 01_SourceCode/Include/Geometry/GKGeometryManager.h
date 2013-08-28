// 负责各种Geometry的创建,
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H
#include "Geometry/GKGeometry.h"

class GEOMETRY_API GKGeometryManager
{
public:
	// 负责创建Geometry
	// nGeoType[in] 要生成的Geometry类型
	GKGeometry* CreateGeometry(GKGeometry::GeoType nGeoType);

};
#endif