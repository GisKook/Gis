// �������.
// ���ݿ���buffer�Ľṹ �洢��С(4bytes)+����(1bytes)+not use(3bytes)+����(�����(4bytes.GKuint32), �㴮ֵ... ...)
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
	// �õ�Geomtry������
	virtual GKBASE::GKuint8 GetType() const {return (GKBASE::GKuint8)GKGeometry::GeoPolygon;} 
};

NAMESPACEEND
#endif