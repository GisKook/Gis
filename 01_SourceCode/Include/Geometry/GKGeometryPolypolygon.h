// 多多边形类
// +每条子线包含的点数(4bytes)+点数(4bytes)+点数据(...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYGON_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYGON_H_H
#include <vector>
#include "Geometry/GKGeometry.h"
NAMESPACEBEGIN(GKGEOMETRY)

class GEOMETRY_API GKGeometryPolypolygon : public GKGeometry
{
public:
	// 得到Geomtry的类型
	virtual GKBASE::GKuint8 GetType() const {return GKGeometry::GeoPolypolygon;}

	void Create(GKBASE::GKuint32 partcount, std::vector<GKBASE::GKuint32> & parts,std::vector<GKBASE::GKPoint2d> & points);

	void SetPartCounts(GKBASE::GKuint32 partcount){m_partcount = partcount;}

	void SetParts(std::vector<GKBASE::GKuint32> & parts){m_parts = parts;}
private: 
	std::vector<GKBASE::GKuint32> m_parts;
	GKBASE::GKuint32 m_partcount; 
};

void GKGeometryPolypolygon::Create( GKBASE::GKuint32 partcount, std::vector<GKBASE::GKuint32> & parts,std::vector<GKBASE::GKPoint2d> & points )
{
	m_partcount = partcount;
	m_parts = parts;
	m_points = points; 
}

NAMESPACEEND
#endif