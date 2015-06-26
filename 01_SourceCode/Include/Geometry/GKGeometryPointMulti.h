#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINTMULTI_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINTMULTI_H_H
#include "Geometry/GKGeometry.h"
#include <vector>

namespace GKGEOMETRY{

class GEOMETRY_API GKGeometryMultiPoint : public GKGeometry
{
public: 
	GKGeometryMultiPoint(){};
	GKGeometryMultiPoint(std::vector<GKBASE::GKPoint2d> & pts){
		m_points = pts;
	}
public:
	// 得到Geomtry的类型
	virtual GKBASE::GKuint8 GetType() const {return (GKBASE::GKuint8)GKGeometry::GeoMultiPoint;} 
};
}
#endif