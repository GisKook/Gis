// 多边形类.
// 数据库中buffer的结构 存储大小(4bytes)+类型(1bytes)+not use(3bytes)+内容(点个数(4bytes.GKuint32), 点串值... ...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYGON_H_H

class GEOMETRY_API GKGeometryPolygon : public GKGeometry
{
public:
	// 得到Geometry存储使用的大小(字节)
	// 总大小+点个数+内容
	virtual GKuint32 GetBufferSize() const {sizeof(GKuint32)+sizeof(GKPoint2d)*m_cPoints.size();}

	// 得到Geomtry的类型
	virtual GKuint8 GetType() const {return (GKuint8)GKGeometry::GeoPolygon;} 

	// 得到点数
	GKuint32 GetPointCounts() const{return m_cPoints.size();}

	// 得到点串的内容
	virtual const GKbyte* GetBuffer() const{return (GKbyte*)&m_cPoints[0]};

	// 设置内容,读取文件或者数据库构造对象
	// param pBuffer[in] buffer
	// param nSize[in] buffer的大小
	// remarks前面四个字节存储的是点数.
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){
		GKuint32 nPointCouts = *pBuffer;
		m_cPoints.resize(nPointCouts);
		memcpy(&m_cPoints[0], pBuffer + 4, nSize - 4);
	};

private:
	std::vector<GKPoint2d> m_cPoints;

};
#endif