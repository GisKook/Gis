// �߶������ݿ��еĴ洢,����ָ������.������size+type+����
// options + line2d
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYLINE_H_H
#include "Geometry/GKGeometry.h"
#include "Geometry/GKGeometryPoint.h"

NAMESPACEBEGIN(GKGEOMETRY)

class GEOMETRY_API GKGeometryLine : public GKGeometry
{
public:
	GKGeometryLine(){};
	GKGeometryLine(GKBASE::GKPoint2d start, GKBASE::GKPoint2d end){m_points.push_back(start); m_points.push_back(end);}
public:
	// �õ�Geomtry������
	virtual GKBASE::GKuint8 GetType() const {return  (GKBASE::GKuint8)GKGeometry::GeoLine;} 
};

NAMESPACEEND
#endif
