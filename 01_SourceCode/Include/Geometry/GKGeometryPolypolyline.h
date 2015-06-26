// ��������,
// buffer�ĽṹΪ:[��С(4bytes)+����(1byte)+not use(2bytes)](Options)+���߸���(4bytes)+ÿ�����ߵĵ���(...)+�㴮(...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYLINE_H_H

namespace GKGEOMETRY{


class GEOMETRY_API GKPolypolyline : public GKGeometry
{
public:
	// �õ�Geometry�洢ʹ�õĴ�С(�ֽ�)
	virtual GKuint32 GetBufferSize() const
	{
		return sizeof(GKulong)+sizeof(GKuint32)+sizeof(GKuint32)*m_nSubLinePointCounts.size()+ sizeof(GKPoint2d)*m_cPoints.size();
	};
	
	// �õ�Geomtry������
	virtual GKuint8 GetType() const {return GKGeometry::GeoPolyPolyline;}

	// �õ����ߵĸ���
	GKuint32 GetSublineCounts()const {return m_nSubLinePointCounts.size(); }

	// �õ�����,����д�뵽�ļ��������ݿ�
	virtual const GKbyte* GetBuffer() const {return (GKbyte*)&m_cPoints[0];}

	// ��������,��ȡ�ļ��������ݿ⹹�����
	// param pBuffer[in] buffer
	// param nSize[in] buffer�Ĵ�С
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize)
	{
		GKASSERT(pBuffer != NULL);
		GKbyte* pTemp = pBuffer;
		GKuint32 nTemp = *pTemp;
		pTemp += 4;
		m_nSubLinePointCounts.resize(nTemp);
		memcpy(&m_nSubLinePointCounts[0], pTemp, nTemp*sizeof(GKuint32));
		pTemp+=nTemp*sizeof(GKuint32);
		nTemp = *pTemp;
		m_cPoints.resize(nTemp);
		pTemp+=4;
		memcpy(&m_cPoints[0], pTemp, nTemp*sizeof(GKPoint2d));
	}
private:
	// �㴮
	std::<GKPoint2d> m_cPoints;
	
	// ÿ������ӵ�еĵ����
	std::<GKuint32> m_nSubLinePointCounts;
};

}
#endif