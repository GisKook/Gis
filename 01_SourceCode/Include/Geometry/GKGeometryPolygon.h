// �������.
// ���ݿ���buffer�Ľṹ �洢��С(4bytes)+����(1bytes)+not use(3bytes)+����(�����(4bytes.GKuint32), �㴮ֵ... ...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H

NAMESPACEBEGIN(GKGEOMETRY)
class GEOMETRY_API GKGeometryPolygon : public GKGeometry
{
public:

public:
	// �õ�Geomtry������
	virtual GKBASE::GKuint8 GetType() const {return (GKBASE::GKuint8)GKGeometry::GeoPolygon;} 

	// �õ�����
	GKBASE::GKuint32 GetPointCounts() const{return m_cPoints.size();}

private:
	std::vector<GKPoint2d> m_cPoints;

};

NAMESPACEEND
#endif