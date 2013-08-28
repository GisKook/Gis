// �������.
// ���ݿ���buffer�Ľṹ �洢��С(4bytes)+����(1bytes)+not use(3bytes)+����(�����(4bytes.GKuint32), �㴮ֵ... ...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H

class GEOMETRY_API GKGeometryPolygon : public GKGeometry
{
public:
	// �õ�Geometry�洢ʹ�õĴ�С(�ֽ�)
	// �ܴ�С+�����+����
	virtual GKuint32 GetBufferSize() const {sizeof(GKuint32)+sizeof(GKPoint2d)*m_cPoints.size();}

	// �õ�Geomtry������
	virtual GKuint8 GetType() const {return (GKuint8)GKGeometry::GeoPolygon;} 

	// �õ�����
	GKuint32 GetPointCounts() const{return m_cPoints.size();}

	// �õ��㴮������
	virtual const GKbyte* GetBuffer() const{return (GKbyte*)&m_cPoints[0]};

	// ��������,��ȡ�ļ��������ݿ⹹�����
	// param pBuffer[in] buffer
	// param nSize[in] buffer�Ĵ�С
	// remarksǰ���ĸ��ֽڴ洢���ǵ���.
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){
		GKuint32 nPointCouts = *pBuffer;
		m_cPoints.resize(nPointCouts);
		memcpy(&m_cPoints[0], pBuffer + 4, nSize - 4);
	};

private:
	std::vector<GKPoint2d> m_cPoints;

};
#endif