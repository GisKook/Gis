// ��������,
// buffer�ĽṹΪ:[��С(4bytes)+����(1byte)+not use(2bytes)](Options)+���߸���(4bytes)+ÿ�����ߵĵ���(...)+�㴮(...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYLINE_H_H
#include "Geometry/GKGeometry.h"

namespace GKGEOMETRY{

class GKGeometryPolyline;

class GEOMETRY_API GKGeometryPolypolyline : public GKGeometry
{
public:
	// �õ�Geomtry������
	virtual GKBASE::GKuint8 GetType() const {return GKGeometry::GeoPolyPolyline;}

	// �õ����ߵĸ���
	GKBASE::GKuint32 GetSublineCounts()const {return m_nSubLinePointCounts.size(); } 

	void SetSubLinePointsCount(std::vector<GKBASE::GKuint32> & sublinepointscount){m_nSubLinePointCounts = sublinepointscount;}

	void AddSubLine(GKGeometryPolyline & subline);
	
	void AddSubLine(GKBASE::GKPolyLine2d & subline);


private:
	// ÿ������ӵ�еĵ����
	std::vector<GKBASE::GKuint32> m_nSubLinePointCounts;
};

}
#endif