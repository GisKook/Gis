// 多折线类,
// buffer的结构为:[大小(4bytes)+类型(1byte)+not use(2bytes)](Options)+子线个数(4bytes)+每条子线的点数(...)+点串(...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYLINE_H_H
#include "Geometry/GKGeometry.h"

namespace GKGEOMETRY{

class GKGeometryPolyline;

class GEOMETRY_API GKGeometryPolypolyline : public GKGeometry
{
public:
	// 得到Geomtry的类型
	virtual GKBASE::GKuint8 GetType() const {return GKGeometry::GeoPolyPolyline;}

	// 得到子线的个数
	GKBASE::GKuint32 GetSublineCounts()const {return m_nSubLinePointCounts.size(); } 

	void SetSubLinePointsCount(std::vector<GKBASE::GKuint32> & sublinepointscount){m_nSubLinePointCounts = sublinepointscount;}

	void AddSubLine(GKGeometryPolyline & subline);
	
	void AddSubLine(GKBASE::GKPolyLine2d & subline);


private:
	// 每个子线拥有的点个数
	std::vector<GKBASE::GKuint32> m_nSubLinePointCounts;
};

}
#endif