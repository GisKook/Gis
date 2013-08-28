// ������,
// ���ݿ���buf�ĽṹΪ:��С[(4bytes)+����(1byte)+not use(2bytes)](Options)+����(4bytes)+������(...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYLINE_H_H

namespace GKGeometryNameSpace{


class GEOMETRY_API GKGeometryPolyline : public GKGeometry
{
public:
	// �õ�Geometry�洢ʹ�õĴ�С(�ֽ�)
	// ��С[(4bytes)+����(1byte)+not use(2bytes)](Options)+���߸���(4bytes)+ÿ�����ߵĵ����(...)+������(...)
	virtual GKuint32 GetBufferSize() const {sizeof(GKuint32)+sizeof(GKPoint2d)*m_cPoints.size();}

	// �õ�Geomtry������
	virtual GKuint8 GetType() const {return (GKuint8)GKGeometry::GeoPolyline;} 

	// �õ�����
	GKuint32 GetPointCounts() const{return m_cPoints.size()};

	// �õ��㴮������
	virtual const GKbyte* GetBuffer() const{return &m_cPoints[0]};

	// ��������,��ȡ�ļ��������ݿ⹹�����
	// param pBuffer[in] buffer
	// param nSize[in] buffer�Ĵ�С
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){
		GKuint32 nPointCouts = *pBuffer;
		m_cPoints.resize(nPointCouts);
		memcpy(&m_cPoints[0], pBuffer + 4, nSize - 4);
	}

private:
	// ������
	std::vector<GKPoint2d> m_cPoints;
};

}
#endif