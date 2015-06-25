// 线段在数据库中的存储,这里指两点线.内容是size+type+内容
// options + line2d
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYLINE_H_H
#include "Geometry/GKGeometry.h"
#include "Geometry/GKGeometryPoint.h"

NAMESPACEBEGIN(GKGEOMERTY)

class GEOMETRY_API GKGeometryLine : public GKGeometry
{
public:
	GKGeometryLine(){};
	GKGeometryLine(GKBASE::GKPoint2d start, GKBASE::GKPoint2d end):m_Line(start, end){};
	GKGeometryLine(GKBASE::GKLine2d line):m_Line(line){};
public:
	// 得到Geomtry的类型
	virtual GKBASE::GKuint8 GetType() const {return  (GKBASE::GKuint8)GKGeometry::GeoLine;} 

private:
	GKBASE::GKLine2d m_Line;
};
NAMESPACEEND
#endif
