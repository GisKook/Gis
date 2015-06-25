// ����
// �����ݿ���ļ��е�洢��Buffer�ṹΪ:size+type+point2d
// options+point2d
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINT_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINT_H_H
#include "Geometry/GKGeometry.h"
#include "Base/GKSystem.h"

namespace GKGEOMERTY{

class GEOMETRY_API GKGeometryPoint : public GKGeometry
{
public:
	GKGeometryPoint(GKBASE::GKdouble x, GKBASE::GKdouble y):m_point(x, y){};
	GKGeometryPoint(GKBASE::GKPoint2d pt):m_point(pt){};
	GKGeometryPoint(){};
public:
	// �õ�����
	virtual GKBASE::GKuint8 GetType() const {return (GKBASE::GKuint8)GKGeometry::GeoPoint;}

private:
	GKBASE::GKPoint2d m_point;
};

}
#endif