// �߶������ݿ��еĴ洢,����ָ������.������size+type+����
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
	// �õ�Geomtry������
	virtual GKBASE::GKuint8 GetType() const {return  (GKBASE::GKuint8)GKGeometry::GeoLine;} 

private:
	GKBASE::GKLine2d m_Line;
};
NAMESPACEEND
#endif
