// 点类
// 在数据库或文件中点存储的Buffer结构为:size+type+point2d
// options+point2d
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINT_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINT_H_H
#include "Geometry/GKGeometry.h"
#include "Base/GKSystem.h"

namespace GKGEOMETRY{

class GEOMETRY_API GKGeometryPoint : public GKGeometry
{
public:
	GKGeometryPoint(GKBASE::GKdouble x, GKBASE::GKdouble y){m_points.push_back(GKBASE::GKPoint2d(x, y));}
	GKGeometryPoint(GKBASE::GKPoint2d pt){m_points.push_back(pt);}
	GKGeometryPoint(){};
public:
	// 得到类型
	virtual GKBASE::GKuint8 GetType() const {return (GKBASE::GKuint8)GKGeometry::GeoPoint;}
	virtual void AddPoint(GKBASE::GKPoint2d pt){m_points[0] = pt;}
	virtual void AddPoint(GKBASE::GKdouble x, GKBASE::GKdouble y){m_points[0] = GKBASE::GKPoint2d(x,y);}
};

}
#endif