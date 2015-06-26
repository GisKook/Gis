// 多边形类.
// 数据库中buffer的结构 存储大小(4bytes)+类型(1bytes)+not use(3bytes)+内容(点个数(4bytes.GKuint32), 点串值... ...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H
#include <vector>
#include "Geometry/GKGeometry.h"

NAMESPACEBEGIN(GKGEOMETRY)
class GEOMETRY_API GKGeometryPolygon : public GKGeometry
{
public: 
	GKGeometryPolygon(){};
	GKGeometryPolygon(std::vector<GKBASE::GKPoint2d> & pts){m_points= pts;};
public:
	// 得到Geomtry的类型
	virtual GKBASE::GKuint8 GetType() const {return (GKBASE::GKuint8)GKGeometry::GeoPolygon;} 
};

NAMESPACEEND
#endif