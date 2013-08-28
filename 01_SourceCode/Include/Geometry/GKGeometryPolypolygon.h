// 多多边形类
// 数据库中buf的结构为:大小[(4bytes)+类型(1byte)+not use(2bytes)](Options)+子线个数(4bytes) 
// +每条子线包含的点数(4bytes)+点数(4bytes)+点数据(...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYGON_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYPOLYGON_H_H

class GEOMETRY_API GKGeometryPolypolygon : public GKGeometry
{
public:
	// 得到Geometry存储使用的大小(字节)
	virtual GKuint32 GetBufferSize() const
	{
		return sizeof(GKuint32)+sizeof(GKuint32)*m_nSubLinePointCounts.size() + sizeof(GKPoint2d)*m_cPoints.size();
	}
	
	// 得到Geomtry的类型
	virtual GKuint8 GetType() const {return GKGeometry::GeoPolypolygon;}

	// 得到内容,用于写入到文件或者数据库
	virtual const GKbyte* GetBuffer() const {return (GKbyte*)&m_cPoints[0];}

	// 设置内容,读取文件或者数据库构造对象
	// param pBuffer[in] buffer
	// param nSize[in] buffer的大小
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
	// 点串
	std::vector<GKPoint2d> m_cPoints;
	
	// 每条子线的点个数
	std::vector<GKuint32> m_nSubLinePointCounts;
};
#endif