// 折线类,
// 数据库中buf的结构为:大小[(4bytes)+类型(1byte)+not use(2bytes)](Options)+点数(4bytes)+点数据(...)
#ifndef GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYLINE_H_H
#define GIS_KOOK_INCLUDE_GEOMETRY_GKGEOMETRYPOLYLINE_H_H

namespace GKGeometryNameSpace{


class GEOMETRY_API GKGeometryPolyline : public GKGeometry
{
public:
	// 得到Geometry存储使用的大小(字节)
	// 大小[(4bytes)+类型(1byte)+not use(2bytes)](Options)+子线个数(4bytes)+每条子线的点个数(...)+点数据(...)
	virtual GKuint32 GetBufferSize() const {sizeof(GKuint32)+sizeof(GKPoint2d)*m_cPoints.size();}

	// 得到Geomtry的类型
	virtual GKuint8 GetType() const {return (GKuint8)GKGeometry::GeoPolyline;} 

	// 得到点数
	GKuint32 GetPointCounts() const{return m_cPoints.size()};

	// 得到点串的内容
	virtual const GKbyte* GetBuffer() const{return &m_cPoints[0]};

	// 设置内容,读取文件或者数据库构造对象
	// param pBuffer[in] buffer
	// param nSize[in] buffer的大小
	virtual void SetBuffer(const GKbyte* pBuffer, GKuint32 nSize){
		GKuint32 nPointCouts = *pBuffer;
		m_cPoints.resize(nPointCouts);
		memcpy(&m_cPoints[0], pBuffer + 4, nSize - 4);
	}

private:
	// 点数据
	std::vector<GKPoint2d> m_cPoints;
};

}
#endif