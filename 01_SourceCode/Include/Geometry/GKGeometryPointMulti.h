#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINTMULTI_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINTMULTI_H_H
#include "Geometry/GKGeometry.h"

namespace GKGEOMERTY{

class GEOMETRY_API GKMultiPoint : public GKGeometry
{
public: 
	GKMultiPoint(){};
	GKMultiPoint(std::vector<GKPoint2d> & pts){
		m_cPoints = pts;
	}
public:
	// 得到Geomtry的类型
	virtual GKBASE::GKuint8 GetType() const {return (GKBASE::GKuint8)GKGeometry::GeoMultiPoint;} 

	// 得到点数
	virtual GKBASE::GKuint32 GetPointCounts() const{return m_cPoints.size();} 

	virtual void AddPoint(GKPoint2d pt){m_cPoints.push_back(pt);}

	GKPoint2d operator[](GKBASE::GKuint32 index){return m_cPoints[index];}
	
private:
	std::vector<GKPoint2d> m_cPoints;

};
}
#endif