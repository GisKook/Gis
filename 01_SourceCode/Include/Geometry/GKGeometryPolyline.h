// ������,
// ���ݿ���buf�ĽṹΪ:��С[(4bytes)+����(1byte)+not use(2bytes)](Options)+����(4bytes)+������(...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYLINE_H_H

#include "Geometry/GKGeometry.h"

namespace GKGEOMETRY{


class GEOMETRY_API GKGeometryPolyline : public GKGeometry
{
public:
	GKGeometryPolyline(){};
	GKGeometryPolyline(std::vector<GKBASE::GKPoint2d> & pts){m_points=pts;};
	GKGeometryPolyline(GKBASE::GKPolyLine2d & line){
		for(GKBASE::GKuint32 i = 0; i < line.nPointCount; ++i){
			m_points.push_back(line.pLinesData[i]);
		}
	}

public:
	// �õ�Geomtry������
	virtual GKBASE::GKuint8 GetType() const {return (GKBASE::GKuint8)GKGeometry::GeoPolyline;} 
};

}
#endif