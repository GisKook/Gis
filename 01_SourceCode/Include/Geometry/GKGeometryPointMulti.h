// ��ʽΪ��С+����+����+�ֽ�����
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINTMULTI_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOINTMULTI_H_H

namespace GKGeometryNameSpace{

class GEOMETRY_API GKMultiPoint : public GKGeometry
{
public:
	// �õ�Geometry�洢ʹ�õĴ�С(�ֽ�)
	// �ܴ�С+�����+����
	virtual GKuint32 GetBufferSize() const {sizeof(GKuint32)+sizeof(GKPoint2d)*m_cPoints.size();}

	// �õ�Geomtry������
	virtual GKuint8 GetType() const {return (GKuint8)GKGeometry::GeoMultiPoint;} 

	// �õ�����
	GKuint32 GetPointCounts() const{return m_cPoints.size();}

	// �õ��㴮������
	virtual const GKbyte* GetBuffer() const{return (GKbyte*)&m_cPoints[0];}
	
	// ����
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){
		GKASSERT(nSize%sizeof(GKPoint2d) == 0 && nSize / sizeof(GKPoint2d) != 0) ;
		GKuint32 nPointCounts = nSize/sizeof(GKPoint2d);
		m_cPoints.resize(nPointCounts);
		memcpy(&m_cPoints[0], pBuffer, nSize);
	};

private:
	std::vector<GKPoint2d> m_cPoints;

};
}
#endif